#ifndef __WEB_H_
#define __WEB_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include "event_loop.h"

#include <vector>

class WEB : public IFdWatcher
{
public:
    WEB();
    ~WEB();

    int start();
    void stop();

    const char* getLocation();

private:
    int  mSock;

    struct sockaddr_in mAddr;
    int    mAddrLen;

    int  getFD();
    bool onFdReadable(int fd);
    int  sendResponse(int sock);
};

#endif /* __WEB_H_ */
