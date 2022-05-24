#ifndef __WIFI_MANAGER_H_
#define __WIFI_MANAGER_H_

#include "task.h"

#include <string>
#include <list>

#include "wpa_ctrl.h"

#include "event_loop.h"
#include "timer.h"
#include <vector>

#include "wifi_event.h"

class IWifiListener
{
public:
    virtual ~IWifiListener() { }

    virtual void onWifiEventReceived(const WifiEvent& event) = 0;
};

class WifiManager : public IFdWatcher, ITimerHandler
{
public:
    static WifiManager& getInstance();

    int  enable();
    void disable();

    int p2pFind(int timeout = -1); /* -1 INFINITE */
    int p2pFind();
    int p2pStopFind();

    int p2pListen(int timeout = -1); /* -1 INFINITE */
    int p2pExtListen(int period, int interval);
    int p2pStopListen();

    int p2pConnect(const std::string& devAddr);
    int setWfdInfo(bool enable, int deviceType, int controlPort, int maxThroughput);

    int p2pRemoveGroup();

    int p2pFlush();

    void addListener(IWifiListener* listener);
    void removeListener(IWifiListener* listener);

private:
    WifiManager();
    ~WifiManager();

    wpa_ctrl* mCtrlConn;
    wpa_ctrl* mMonitorConn;

    typedef std::list<IWifiListener*> WifiListenerList;
    WifiListenerList mWifiListeners;

private:
    /* Override from IFdWatcher */
    int  getFD();
    bool onFdReadable(int fd);
    
    /* Override from ITimerHandler */
    void onTimerExpired(const ITimer* timer);

    void startSupplicant();
    void stopSupplicant();

    int connectToSupplicant(int tryCnt);
    int connectToSockPath(const std::string& path);
    
    void disconnectToSupplicant();

    void init();

    void notify(const WifiEvent& event);

    std::string sendCommand(const std::string& cmd);
    int sendCommand(const char* cmd, char* reply, size_t* reply_len);
};

#endif /* __WIFI_SERVICE_H_ */
