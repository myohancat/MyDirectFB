#include "web.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

#include "netutil.h"
#include "trim.h"
#include "log.h"

#define LOCATION_PORT 1901
#define LOCATION_DESC "/dd.xml"

// TODO MUST IMPLEMENTS HERE
#define FRIENDLY_NAME   "Myohancat"
#define COMPANY         "myocat.iptime.org"
#define MODEL_NAME      "Myocat"
#define UUID     "deadbeef-dead-beef-dead-beefdeadbeef" // DeviceConfig::getInstance().getUUID()

static const char* FMT_DD_XML = "<?xml version=\"1.0\"?>"
                                "<root"
                                " xmlns=\"urn:schemas-upnp-org:device-1-0\""
                                  " xmlns:r=\"urn:restful-tv-org:schemas:upnp-dd\">"
                                " <specVersion>"
                                " <major>1</major>"
                                " <minor>0</minor>"
                                " </specVersion>"
                                " <device>"
                                " <deviceType>urn:schemas-upnp-org:device:tvdevice:1</deviceType>"
                                " <friendlyName>%s</friendlyName>"
                                " <manufacturer>%s</manufacturer>"
                                " <modelName>%s</modelName>"
                                " <UDN>uuid:%s</UDN>"
                                " </device>"
                                "</root>";

static const char* MSG_400_BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
static const char* MSG_404_NOT_FOUND   = "HTTP/1.1 404 Not Found\r\n";

WEB::WEB()
{

}

int WEB::start()
{
    mSock = socket(AF_INET, SOCK_STREAM, 0);
    if(mSock < 0)
    {   
        LOG_ERROR("bind error \n");
    }

    memset(&mAddr, 0x0, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    mAddr.sin_port = htons(LOCATION_PORT); 
    mAddrLen = sizeof(mAddr);

    int option = 1;
    setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if(bind(mSock, (struct sockaddr*) &mAddr, mAddrLen) < 0 )
    {
        LOG_ERROR("bind error. err=%s(%d)\n", strerror(errno), errno);
        close(mSock);
        return -1;
    }
    
    if(listen(mSock, 2 ) == -1 )
    {
        LOG_ERROR("listen error. err=%s(%d)\n", strerror(errno), errno);
        close(mSock);
        return -1;
    }

    EventLoop::getInstance().addFdWatcher(this);

    LOG_INFO("Start WEB SUCCESS !!!!!\n");    
    return 0;
}

void WEB::stop()
{
    EventLoop::getInstance().removeFdWatcher(this);

    if(mSock > 0)
        close(mSock);

    LOG_INFO("Stop WEB SUCCESS !!!!!\n");    
}

WEB::~WEB()
{
}

int WEB::getFD()
{
    return mSock;
}

const char* WEB::getLocation()
{
    static char location[1024];

    char host_ip[36];
    NetUtil::get_ip_addr("eth0", host_ip);

    sprintf(location, "http://%s:%d%s", host_ip, LOCATION_PORT, LOCATION_DESC);

    return location;
}

int  WEB::sendResponse(int sock)
{
    char msg[2048];

    const char* head = "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/xml\r\n"
                 "Connection: close\r\n"
                 "\r\n";

    if (send(sock, head, strlen(head), 0) < 0)
    {
        LOG_ERROR("Send header is failed !\n");
        return -1;
    }

    snprintf(msg, sizeof(msg), FMT_DD_XML,
        FRIENDLY_NAME, /* friendly name */
        COMPANY, /* manufacturer */
        MODEL_NAME, /* model name */
        UUID /* UUID */
    );

    if (send(sock, msg, strlen(msg), 0) < 0)
    {
        LOG_ERROR("Send header is failed !\n");
        return -1;
    }

    return 0;
}

bool WEB::onFdReadable(int fd)
{
    int rsize;
    char msg[2*1024];
    char* reqline[3];

    int clntSock = accept(mSock, (struct sockaddr*) &mAddr, (socklen_t *) &mAddrLen);
    if(clntSock < 0)
        return true;

    LOG_INFO("Connected Client !\n");
    memset(msg, 0x00, sizeof(msg));    
    rsize = recv(fd, msg, sizeof(msg) - 1,  0);
    if (rsize <= 0)
    {
        LOG_ERROR("recv failed ! error : %s\n", strerror(errno));
        goto ERROR;
    }
    msg[rsize] = 0;

    /* Received header and check valid */
    reqline[0] = strtok(msg, " \t\r\n");
    if (strncmp(reqline[0], "GET", 4) == 0)
    {
        reqline[1] = strtok(NULL, " \t");
        reqline[2] = strtok(NULL, " \t\r\n");

        /* Check Request HEADER */
        if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0)
        {
            if (write(clntSock, MSG_400_BAD_REQUEST, strlen(MSG_400_BAD_REQUEST)) < 0)
            {
                LOG_ERROR("Failed to response 400 failed ! error : %s\n", strerror(errno));
                goto ERROR;
            }
            goto EXIT;
        }

        const char* desc = trim(reqline[1]);
        if (strcmp(desc, LOCATION_DESC) != 0)
        {
            if (write(clntSock, MSG_404_NOT_FOUND, strlen(MSG_404_NOT_FOUND)) < 0)
            {
                LOG_ERROR("Failed to response 404 failed ! error : %s\n", strerror(errno));
                goto ERROR;
            }
            goto EXIT;
        }

        if (sendResponse(clntSock) != 0)
        {
            LOG_ERROR("Send response failed !\n");
            goto ERROR;
        }
    }

    goto EXIT;
ERROR:
    shutdown(clntSock, SHUT_RDWR);

EXIT:
    close(clntSock);

    return true;
}
