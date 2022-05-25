#ifndef __SSDP_H_
#define __SSDP_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include "web.h"
#include "event_loop.h"

#include <vector>

#define MULTICAST_ADDR  "239.255.255.250"
#define MULTICAST_PORT  1900

class SSDP : public IFdWatcher, ITimerHandler
{
    typedef enum
    {
        NOTIFY_TYPE_ALIVE,
        NOTIFY_TYPE_BYEBYE,

        MAX_NOTIFY_TYPE
    }NotifyType_e;

public:
    static SSDP& getInstance();
    static void destroyInstance();

    int start();
    void stop();

private:
    int                mSock;
    struct sockaddr_in mAddr;
    int                mAddrLen;
    WEB                mWeb;

    Timer              mTimer;

    SSDP();
    ~SSDP();

    int  getFD();
    bool onFdReadable(int fd);
    void onTimerExpired(const ITimer* timer);
    
    int parseSSDP(struct sockaddr_in* sa, socklen_t salen, char* payload);
    int replyMSearch(struct sockaddr_in* sa, socklen_t salen, const char* st);
    int sendNotify(NotifyType_e eType);
};

#endif /* __SSDP_H_ */
