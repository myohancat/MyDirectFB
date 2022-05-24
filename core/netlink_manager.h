#ifndef __NETLINK_MANAGER_H_
#define __NETLINK_MANAGER_H_

#include <linux/netlink.h>

#include "event_loop.h"
#include "timer.h"
#include <vector>

class INetlinkListener
{
public:
    virtual ~INetlinkListener() { }

    virtual void onRouteChanged(bool isNew, const char* ifname, const char* destaddr, const char* gateway) = 0;
    virtual void onLinkChanged(bool isNew, const char* ifname) = 0;
    virtual void onAddressChanged(bool isNew, const char* ifname, const char* ifaddr) = 0;
};

class NetlinkManager : public IFdWatcher, ITimerHandler
{
public:
    static NetlinkManager& getInstance();

    void addListener(INetlinkListener* observer);
    void removeListener(INetlinkListener* observer);

private:
    int mSock;
    Timer mTimer;

    std::vector<INetlinkListener*> mListeners;

private:
    NetlinkManager();
    ~NetlinkManager();

    int  getFD();
    bool onFdReadable(int fd);
    
    /* Polling to check IP */
    void onTimerExpired(const ITimer* timer);

    void parseRouteMsg(struct nlmsghdr *h);
    void parseLinkMsg(struct nlmsghdr *h);
    void parseAddressMsg(struct nlmsghdr *h);
};


#endif /* __NETLINK_MANAGER_H_ */
