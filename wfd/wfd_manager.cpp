#include "wfd_manager.h"

#include "log.h"

#include <algorithm>

#define DEFAULT_DEVICE_TYPE    (0x11) // PRIMARY_SINK_OR...
#define DEFAULT_CONTROL_PORT   (7236) 
#define DEFAULT_MAX_THROUGHPUT (50)

typedef enum
{
    WFD_EVENT_CONNECTION_RECV,
    WFD_EVENT_CONNECTION_FAILED,
    WFD_EVENT_CONNECTED,
    WFD_EVENT_DISCONNECTED,
    WFD_EVENT_PLAY_STARTED,
    
    WFD_EVENT_INVALID
} WFD_EVENT_ID;

WfdManager& WfdManager::getInstance()
{
    static WfdManager instance;

    return instance;
}

WfdManager::WfdManager()
           : mWfdSource(NULL),
             mGroup(NULL),
             mRtspSession(NULL),
             mRtpPlayer(NULL)
{
    mDhcpTimer.setHandler(this);
}

WfdManager::~WfdManager()
{

}

int WfdManager::enable()
{
    WifiManager::getInstance().addListener(this);
    NetlinkManager::getInstance().addListener(this);

    WifiManager::getInstance().p2pFlush();
    WifiManager::getInstance().setWfdInfo(true, DEFAULT_DEVICE_TYPE, DEFAULT_CONTROL_PORT, DEFAULT_MAX_THROUGHPUT);
    WifiManager::getInstance().p2pListen();

    return 0;
}

void WfdManager::disable()
{
    mDhcpTimer.stop();

    WifiManager::getInstance().setWfdInfo(false, 0, 0, 0);
    WifiManager::getInstance().p2pStopFind();

    NetlinkManager::getInstance().removeListener(this);
    WifiManager::getInstance().removeListener(this);
}

void WfdManager::onWifiEventReceived(const WifiEvent& event)
{
    if (event.getType() == WifiEvent::EVENT_TYPE_P2P_DEVICE_FOUND)
    {
        WifiP2pDevice* dev = (WifiP2pDevice*)event.getExtraData();

        if (dev->getWfdInfo())
            addP2pDevice(*dev);
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_DEVICE_LOST)
    {
        WifiP2pDevice* dev = (WifiP2pDevice*)event.getExtraData();

        removeP2pDevice(*dev);
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_PROV_DISC_PBC_REQ)
    {
        WifiP2pDevice* dev = (WifiP2pDevice*)event.getExtraData();

        LOG_INFO("EVENT_TYPE_P2P_PROV_DISC_PBC_REQ : %s(%s)\n", dev->getDeviceAddress().c_str(), dev->getDeviceName().c_str());
        
        // TODO IMPLEMENTS HERE
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_GO_NEG_SUCCESS)
    {
        // TODO IMPELMENTS HERE
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_INVITATION_ACCEPTED)
    {
        std::string* addr = (std::string*)event.getExtraData();

        LOG_INFO("WifiEvent::EVENT_TYPE_P2P_INVITATION_ACCEPTED : %s\n", addr->c_str());
        WifiManager::getInstance().p2pStopFind();
    
        WifiP2pDevice* dev = findP2pDevice(*addr);
        if (dev)
        {
            LOG_INFO("save wifi p2p device !\n");
            mWfdSource = new WifiP2pDevice(*dev);
            notifyEvent(WFD_EVENT_CONNECTION_RECV, mWfdSource);
        }
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_GO_NEG_REQUEST)
    {
        WifiP2pConfig* config = (WifiP2pConfig*)event.getExtraData();

        LOG_INFO("EVENT_TYPE_P2P_GO_NEG_REQUEST : %s\n", config->getDeviceAddress().c_str());

        WifiManager::getInstance().p2pStopFind();

        if (mWfdSource)
        {
            LOG_WARN("Wfd Source is already exist. %s(%s)\n", mWfdSource->getDeviceName().c_str(), mWfdSource->getDeviceAddress().c_str());
            delete mWfdSource;
            mWfdSource = NULL;
        }

        WifiP2pDevice* dev = findP2pDevice(config->getDeviceAddress());
        if (!dev)
        {
            LOG_ERROR("Cannot found p2p device : %s\n", config->getDeviceAddress().c_str());
            return;
        }

        if (WifiManager::getInstance().p2pConnect(dev->getDeviceAddress()) == 0)
        {
            LOG_INFO("save wifi p2p device !\n");
            mWfdSource = new WifiP2pDevice(*dev);
            notifyEvent(WFD_EVENT_CONNECTION_RECV, mWfdSource);
        }
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_GROUP_STARTED)
    {
        WifiP2pGroup* group = (WifiP2pGroup*)event.getExtraData();
        LOG_INFO("EVENT_TYPE_P2P_GROUP_STARTED : %s - %s\n", group->getInterface().c_str(), group->getOwnerAddress().c_str());
        if (mGroup)
        {
            LOG_WARN("P2P Group is already exist. %s(%s)\n", group->getNetworkName().c_str(), group->getInterface().c_str());
            delete mGroup;
        }
        
        mGroup = new WifiP2pGroup(*group);
        if (!mWfdSource)
        {
            WifiP2pDevice* dev = findP2pDevice(mGroup->getOwnerAddress());
            if (!dev)
            {
                LOG_ERROR("Cannot found p2p device : %s\n", mGroup->getOwnerAddress().c_str());
                return;
            }

            LOG_WARN("XXXX save wifi p2p device !\n");
            mWfdSource = new WifiP2pDevice(*dev);
            notifyEvent(WFD_EVENT_CONNECTION_RECV, mWfdSource);
        }

        notifyEvent(WFD_EVENT_CONNECTED, mWfdSource);
    }
    else if (event.getType() == WifiEvent::EVENT_TYPE_P2P_GROUP_REMOVED)
    {
        LOG_INFO("EVENT_TYPE_P2P_GROUP_REMOVED\n");
        if (mWfdSource)
        {
            notifyEvent(WFD_EVENT_DISCONNECTED, mWfdSource);
            delete mWfdSource;
            mWfdSource = NULL;
        }

        if (mGroup)
        {
            delete mGroup;
            mGroup = NULL;
        }

        if (mRtspSession)
        {
            delete mRtspSession;
            mRtspSession = NULL;
        }

        WifiManager::getInstance().p2pListen();
    }
}

void WfdManager::onRouteChanged(bool isNew, const char* ifname, const char* destaddr, const char* gateway)
{
    if (isNew && mGroup && mGroup->getInterface() == ifname)
    {
        LOG_INFO("[%s] %s, DSTADDR : %s, GW : %s\n", isNew?"NEW":"DEL", ifname, destaddr, gateway);

        if (gateway)
        {
            if (!mWfdSource)
            {
                LOG_ERROR("WFD Source is not selected.! Skip make session !\n");
                return;
            }

            WfdInfo_t* info = mWfdSource->getWfdInfo();
            if (!info)
            {
                LOG_ERROR("This source is not WifiDisplay Device. Skip it !\n");
                return;
            }

            if (mRtspSession)
            {
                LOG_WARN("RTSP Session already exsit..!\n");
                return;
            }

            mRtspSession = new RtspClient();
            mRtspSession->addListener(this);
            mRtspSession->start(gateway, info->mCtrlPort);
        }
    }
}

void WfdManager::onLinkChanged(bool isNew, const char* ifname)
{
    if (isNew && mGroup && mGroup->getInterface() == ifname)
    {
        LOG_INFO("[%s] %s\n", isNew?"UP":"DOWN", ifname);

        mInterface = ifname;
        mDhcpTimer.stop();
        mDhcpTimer.start(1000, false);
    }
}

void WfdManager::onAddressChanged(bool isNew, const char* ifname, const char* ifaddr)
{
}

void WfdManager::onRtspConnected()
{
    LOG_DEBUG("[[[[RTSP Connected ]]]]\n");
}

void WfdManager::onRtspFailed(int error)
{
    LOG_DEBUG("[[[[ RTSP Failed : %d ]]]]\n", error);
}

void WfdManager::onRtspClosed()
{
    LOG_DEBUG("[[[[ RTSP Closed ]]]]\n");
    if (mRtpPlayer)
    {
        delete mRtpPlayer;
        mRtpPlayer = NULL;
    }
}

void WfdManager::onRtspSessionStarted()
{
    LOG_DEBUG("[[[[ RTSP Session Started ]]]]\n");
    if (!mRtpPlayer)
    {
        mRtpPlayer = new SimpleRtpPlayer();
    }

    notifyEvent(WFD_EVENT_PLAY_STARTED, mWfdSource);
    mRtpPlayer->start(19000);
}

void WfdManager::onRtspSessionEnded()
{
    LOG_DEBUG("[[[[ RTSP Session Ended ]]]]\n");
    if (mRtpPlayer)
    {
        mRtpPlayer->stop();
    }
}

void WfdManager::addP2pDevice(const WifiP2pDevice& device)
{
    P2pDeviceList::iterator it = std::find(mP2pDevices.begin(), mP2pDevices.end(), device);
    if(device == *it)
    {
        LOG_DEBUG("P2pDevice is alreay exsit !!\n");
        return;
    }

    mP2pDevices.push_back(device);
}

void WfdManager::removeP2pDevice(const WifiP2pDevice& device)
{
    for(P2pDeviceList::iterator it = mP2pDevices.begin(); it != mP2pDevices.end(); it++)
    {
        if(*it == device)
        {
            mP2pDevices.erase(it);
            return;
        }
    }
}

WifiP2pDevice* WfdManager::findP2pDevice(const std::string& devAddress)
{
    for(P2pDeviceList::iterator it = mP2pDevices.begin(); it != mP2pDevices.end(); it++)
    {
        if((*it).getDeviceAddress() == devAddress)
        {
            return &(*it);
        }
    }

    return NULL;
}

void WfdManager::removeP2pDeviceAll()
{
    mP2pDevices.clear();
}


void WfdManager::onTimerExpired(const ITimer* timer)
{
    if (timer == &mDhcpTimer)
    {
#if 0 /* Raspberry pi doesn't need to udhcpc */
        char cmd[1024];

        sprintf(cmd, "udhcpc -i %s", mInterface.c_str());
        system(cmd);
#endif
    }
}

void WfdManager::addListener(IWfdEventListener* listener)
{
    if(!listener)
        return;

    WfdEventListenerList::iterator it = std::find(mWfdEventListeners.begin(), mWfdEventListeners.end(), listener);
    if(listener == *it)
    {
        LOG_ERROR("WfdEventListener is alreay exsit !!\n");
        return;
    }
    mWfdEventListeners.push_front(listener);
}

void WfdManager::removeListener(IWfdEventListener* listener)
{
    if(!listener)
        return;

    for(WfdEventListenerList::iterator it = mWfdEventListeners.begin(); it != mWfdEventListeners.end(); it++)
    {
        if(listener == *it)
        {
            mWfdEventListeners.erase(it);
            return;
        }
    }
}

void WfdManager::notifyEvent(int eventId, void* param1, void* param2)
{
    for(WfdEventListenerList::iterator it = mWfdEventListeners.begin(); it != mWfdEventListeners.end(); it++)
    {
        IWfdEventListener* listener = *it;

        switch(eventId)
        {
            case WFD_EVENT_CONNECTION_RECV:
            {
                WifiP2pDevice* dev = (WifiP2pDevice*)param1;
                listener->onConnectionReceived(dev->getDeviceName());
                break;
            }
            case WFD_EVENT_CONNECTION_FAILED:
            {
                WifiP2pDevice* dev = (WifiP2pDevice*)param1;
                int error          = (int)param2;

                listener->onConnectionFailed(dev->getDeviceName(), error);
                break;
            }
            case WFD_EVENT_CONNECTED:
            {
                WifiP2pDevice* dev = (WifiP2pDevice*)param1;
                listener->onConnected(dev->getDeviceName());
                break;
            }
            case WFD_EVENT_DISCONNECTED:
            {
                WifiP2pDevice* dev = (WifiP2pDevice*)param1;
                listener->onDisconnected(dev->getDeviceName());
                break;
            }
            case WFD_EVENT_PLAY_STARTED:
            {
                listener->onPlayStarted();
                break;
            }
        }
    }
}
