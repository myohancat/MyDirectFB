#ifndef __WFD_MANAGER_H_
#define __WFD_MANAGER_H_

#include "timer.h"

#include "wifi_manager.h"
#include "netlink_manager.h"
#include "rtsp_client.h"
#include "simple_rtp_player.h"

class IWfdEventListener
{
public:
    virtual ~IWfdEventListener() { }

    virtual void onConnectionReceived(const std::string& device) = 0;
    virtual void onConnectionFailed(const std::string& device, int error) = 0;
    virtual void onConnected(const std::string& device) = 0;
    virtual void onDisconnected(const std::string& device) = 0;

    virtual void onPlayStarted() = 0;
    virtual void onPlayStopped() = 0;
};

class WfdManager : public IWifiListener, INetlinkListener, ITimerHandler, IRtspClientListener
{
public:
    static WfdManager& getInstance();

    int  enable();
    void disable();

    void addListener(IWfdEventListener* listener);
    void removeListener(IWfdEventListener* listener);

private:
    WifiP2pDevice*   mWfdSource;
    WifiP2pGroup*    mGroup;

    RtspClient*      mRtspSession;
    SimpleRtpPlayer* mRtpPlayer;

    std::string      mInterface;
    Timer            mDhcpTimer;

    typedef std::list<WifiP2pDevice> P2pDeviceList;
    P2pDeviceList mP2pDevices;

    typedef std::list<IWfdEventListener*> WfdEventListenerList;
    WfdEventListenerList mWfdEventListeners;

private:
    WfdManager();
    ~WfdManager();

    void addP2pDevice(const WifiP2pDevice& device);
    void removeP2pDevice(const WifiP2pDevice& device);
    void removeP2pDeviceAll();
    WifiP2pDevice* findP2pDevice(const std::string& devAddress);

    void onWifiEventReceived(const WifiEvent& event);

    void onRouteChanged(bool isNew, const char* ifname, const char* destaddr, const char* gateway);
    void onLinkChanged(bool isNew, const char* ifname);
    void onAddressChanged(bool isNew, const char* ifname, const char* ifaddr);

    void onRtspConnected();
    void onRtspFailed(int error);
    void onRtspClosed();
    void onRtspSessionStarted();
    void onRtspSessionEnded();

    void onTimerExpired(const ITimer* timer);

    void notifyEvent(int eventId, void* param1 = NULL, void* param2 = NULL);
};

#endif /* __WFD_MANAGER_H_ */
