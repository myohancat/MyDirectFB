#include "ssdp.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include "log.h"
#include "trim.h"

#define MAX_PACKET_SIZE    (2*1024)
#define SERVER_NAME  "Linux/4.19.66-v7, UPnP/1.0 MyApp/Device" // TODO
#define SERVICE_TYPE "urn:myocat.iptime.org:my_app:1" // TODO 
#define CACHE_TIMEOUT 120 // 1800

// TODO MUST IMPLEMENTS HERE
#define UUID     "deadbeef-dead-beef-dead-beefdeadbeef" // DeviceConfig::getInstance().getUUID()

static const char* gSupportedServiceTypes[] = 
{
    "ssdp:all",
    "upnp:rootdevice",
    SERVICE_TYPE,
    NULL
};

static const char* FMT_MSEACH_REPLY = "HTTP/1.1 200 OK\r\n"
                                      "CACHE-CONTROL: max-age=%d\r\n"
                                      "DATE: %s\r\n"
                                      "EXT:\r\n"
                                      "LOCATION: %s\r\n"
                                      "SERVER: %s\r\n"
                                      "ST: %s\r\n"
                                      "USN: uuid:%s::"
                                      "%s\r\n"
                                      "\r\n";

static const char* FMT_NOTIFY_ALIVE = "NOTIFY * HTTP/1.1\r\n" 
                                      "HOST: %s:%d\r\n" 
                                      "CACHE-CONTROL: max-age=%d\r\n"
                                      "LOCATION: %s\r\n"
                                      "NT: %s\r\n"
                                      "NTS: ssdp:alive\r\n"
                                      "SERVER: %s\r\n"
                                      "USN: uuid:%s\r\n"
                                      "CONTENT-LENGTH: 0\r\n"
                                      "\r\n";

static const char* FMT_NOTIFY_BYEBYE = "NOTIFY * HTTP/1.1\r\n"
                                       "HOST: %s:%d\r\n"
                                       "NT: %s\r\n"
                                       "NTS: ssdp:byebye\r\n"
                                       "USN: uuid:%s\r\n"
                                       "\r\n";

static SSDP* _instance = NULL;

SSDP& SSDP::getInstance()
{
    if (!_instance)
        _instance = new SSDP();

    return *_instance;
}

void SSDP::destroyInstance()
{
    if (_instance)
    {
        delete _instance;
        _instance = NULL;
    }
}

SSDP::SSDP()
{
    mTimer.setHandler(this);
}

int SSDP::start()
{
    int opt = 1;
    int ttl = 3;
    struct ip_mreq mreq;

    memset(&mreq, 0x00, sizeof(struct ip_mreq));

    mSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(mSock < 0)
    {
        mSock = -1;
        LOG_ERROR("socket error (%d) - %s\n", errno, strerror(errno));
        return -1;
    }

    if(setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(mSock);
        LOG_ERROR("cannot set SO_REUSEADDR\n");
        return -1;
    }

    if(setsockopt(mSock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
        close(mSock);
        LOG_ERROR("cannot set multicast TTL: %s\n", strerror(errno));
        return -1;
    }

    memset(&mAddr, 0x00, sizeof(mAddr));
    mAddr.sin_family   = AF_INET;
    inet_aton(MULTICAST_ADDR, &mAddr.sin_addr);
    mAddr.sin_port     = htons(MULTICAST_PORT);
    mAddrLen = sizeof(mAddr);

    if(bind(mSock, (struct sockaddr *)&mAddr, mAddrLen) < 0)
    {
        close(mSock);
        LOG_ERROR("bind error (%d) - %s\n", errno, strerror(errno));
        return -1;
    }

    /* Join a mulitcast group */
    //inet_aton(MULTICAST_ADDR, &mreq.imr_multiaddr);
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); /* TBD. CHECK THIS */
    //mreq.imr_interface.s_addr = inet_addr("172.16.0.102");

    if(setsockopt(mSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof(mreq)) < 0)
    {
        close(mSock);
        LOG_ERROR("setsockopt error (%d) - %s\n", errno, strerror(errno));
        return -1;
    }
    
    EventLoop::getInstance().addFdWatcher(this);
    mWeb.start();

    sendNotify(NOTIFY_TYPE_ALIVE);
    mTimer.start((CACHE_TIMEOUT / 2) * 1000, true);

    return 0;
}

void SSDP::stop()
{
    mTimer.stop();
    mWeb.stop();

    EventLoop::getInstance().removeFdWatcher(this);

    sendNotify(NOTIFY_TYPE_BYEBYE);
    if(mSock > 0)
        close(mSock);
}

SSDP::~SSDP()
{
}

int SSDP::getFD()
{
    return mSock;
}

int SSDP::parseSSDP(struct sockaddr_in* sa, socklen_t salen, char* payload)
{
    char* saveptr;
    char* line;

    const char* st = NULL;

    LOG_DEBUG(payload);
    line = strtok_r(payload, "\n", &saveptr);

    if(strncmp(line, "M-SEARCH", strlen("M-SEARCH")))
    {
        // NO NEED TO CHECK
        return 0;
    }
    
    while((line = strtok_r(NULL, "\n", &saveptr)))
    {
        if(!strncasecmp(line, "ST:", strlen("ST:")))
        {
            char* ptr = line + strlen("ST:");
            st = trim(ptr);
        }
    }

    if (st == NULL)
        st = "ssdp:all";

    for (int ii = 0; gSupportedServiceTypes[ii]; ii++)
    {
        if (!strcmp(gSupportedServiceTypes[ii], st))
        {
            LOG_INFO("found service type : %s\n", gSupportedServiceTypes[ii]);
            replyMSearch(sa, salen, st);
        }
    }

    return 0;
}

int SSDP::replyMSearch(struct sockaddr_in* sa, socklen_t salen, const char* st)
{
    int ret;

    //struct sockaddr_in dest;
    char buf[MAX_PACKET_SIZE];
    //char usn[256];
    char date[42];
    time_t now;

    now = time(NULL);
    strftime(date, sizeof(date), "%a, %d %b %Y %T %Z", gmtime(&now));
    
    LOG_INFO("try make M-Search Reply\n");
    snprintf(buf, sizeof(buf), FMT_MSEACH_REPLY,
            CACHE_TIMEOUT,
            date,
            mWeb.getLocation(),
            SERVER_NAME,
            st,
            UUID,
            st 
            );
    
    LOG_INFO("%s", buf);

    ret = sendto(mSock, buf, strlen(buf), 0, (struct sockaddr *)sa, salen);
    if (ret < 0)
        LOG_ERROR("Failed sending SSDP M-SEARCH reply, error : %s\n", strerror(errno));

    return 0;
}

int SSDP::sendNotify(NotifyType_e eType)
{
    int  ret;
    char buf[2* 1024];
    char usn[256];

    if (mSock < 0)
        return -1;


    snprintf(usn, sizeof(usn), "%s::%s", UUID, SERVICE_TYPE);

    if (eType == NOTIFY_TYPE_ALIVE)
    {
        snprintf(buf, sizeof(buf), FMT_NOTIFY_ALIVE,
            MULTICAST_ADDR, MULTICAST_PORT, /* HOST */
            CACHE_TIMEOUT, /* CACHE-CONTROL */
            mWeb.getLocation(), /* LOCATION */
            SERVICE_TYPE, /* NT */
            SERVER_NAME, /* SERVER */
            usn /* USN */
            );
    }
    else // if (eType == NOTIFY_TYPE_BYEBYE)
    {
        snprintf(buf, sizeof(buf), FMT_NOTIFY_BYEBYE,
            MULTICAST_ADDR, MULTICAST_PORT, /* HOST */
            SERVICE_TYPE, /* NT */
            usn /* USN */
            );
    }

    ret = sendto(mSock, buf, strlen(buf), 0, (struct sockaddr*)&mAddr, mAddrLen);
    if (ret < 0)
    {
        LOG_ERROR("Cannot send notify : %s, error : %s\n", (eType == NOTIFY_TYPE_ALIVE) ? "Alive" : "Byebye", strerror(errno));
        return -1;
    }

    return 0;
}

bool SSDP::onFdReadable(int fd)
{
    int ret;
    char buf[4096];
    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);

    ret = recvfrom(fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&caddr, &caddrlen);

    if(ret > 0)
    {
        char ip[128];

        buf[ret] = 0;
        inet_ntop(AF_INET, &(caddr.sin_addr), ip, INET_ADDRSTRLEN);
        
        parseSSDP(&caddr, caddrlen, buf);
    }

    return true;
}

void SSDP::onTimerExpired(const ITimer* timer)
{
    UNUSED(timer);
    // TBD. IMPLEMENTS HERE
    sendNotify(NOTIFY_TYPE_ALIVE);
}
