#include "wifi_manager.h"

#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

#include <algorithm>

#include "log.h"

#define WLAN_IFACE_NAME "wlan0"
#define P2P_IFACE_NAME  "p2p-dev-wlan0"

#define WPA_SUPPLICANT_CONF_FILE_PATH "/etc/wpa_supplicant.conf"
#define WPA_SUPPLICANT_PID_FILE_PATH  "/var/run/wpa_supplicant.pid"
#define P2P_SUPPLICANT_CONF_FILE_PATH "/etc/p2p_supplicant.conf"
#define WPA_SUPPLICANT_GLOBAL_IFACE   "/var/run/wpa-wlan0"
#define WPA_SUPPLICANT_CTRL_INTERFACE "/var/run/wpa_supplicant"

static bool isSupplicantRunnig()
{
    size_t ret;
    int    pid;
    char buf[64];

    FILE* fp = fopen(WPA_SUPPLICANT_PID_FILE_PATH, "r");
    if (fp == NULL)
        return false;

    ret = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);
    if (ret < 0)
        return false;

    buf[ret] = 0;
    pid = strtol(buf, NULL, 10);
    snprintf(buf, sizeof(buf), "/proc/%d", pid);
    buf[sizeof(buf) -1] = 0;

    return (access(buf, F_OK) == 0);
}

WifiManager& WifiManager::getInstance()
{
    static WifiManager instance;

    return instance;
}

WifiManager::WifiManager()
            : mCtrlConn(NULL),
              mMonitorConn(NULL)
{
    startSupplicant();
}

WifiManager::~WifiManager()
{
}

int WifiManager::enable()
{
__TRACE_FUNC__;
    if (connectToSupplicant(10) != 0)
        return -1;

    EventLoop::getInstance().addFdWatcher(this);
    init();

    return 0;
}

void WifiManager::disable()
{
__TRACE_FUNC__;
    EventLoop::getInstance().removeFdWatcher(this);
    disconnectToSupplicant();
}

int WifiManager::p2pFind(int timeout)
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_FIND ";
    if (timeout > 0)
    {
        char arg[32];
        sprintf(arg, "%d", timeout);
        cmd.append(arg);
    }

    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to p2p find\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pStopFind()
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_STOP_FIND";

    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to p2p stop find\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pListen(int timeout)
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_LISTEN ";
    if (timeout > 0)
    {
        char arg[32];
        sprintf(arg, "%d", timeout);
        cmd.append(arg);
    }

    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to p2p listen\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pExtListen(int period, int interval)
{
__TRACE_FUNC__;
    return 0;
}

int WifiManager::p2pStopListen()
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_EXT_LISTEN ";
    
    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to p2p stop listen\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pConnect(const std::string& devAddr)
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_CONNECT " + devAddr + " pbc persistent go_intent=0";

    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to connect p2p : %s\n", devAddr.c_str());
        return -1;
    }
    
    return 0;
}

int WifiManager::setWfdInfo(bool enable, int deviceType, int controlPort, int maxThroughput)
{
__TRACE_FUNC__;
    char cmd[128];
    std::string reply;

    sprintf(cmd, "SET wifi_display %d", (int)enable);

    reply = sendCommand(cmd);

    if (enable)
        sprintf(cmd, "WFD_SUBELEM_SET 0 %04x%04x%04x%04x", 6, deviceType, controlPort, maxThroughput);
    else
        strcpy(cmd, "WFD_SUBELEM_SET 0 0");

    reply = sendCommand(cmd);

    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to set wfdinfo\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pRemoveGroup()
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_GROUP_REMOVE";
    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to p2p remove group\n");
        return -1;
    }

    return 0;
}

int WifiManager::p2pFlush()
{
__TRACE_FUNC__;
    std::string cmd = "IFNAME=" P2P_IFACE_NAME " P2P_FLUSH";

    std::string reply = sendCommand(cmd);
    if (reply.compare("OK\n") != 0)
    {
        LOG_ERROR("failed to flush p2p\n");
        return -1;
    }

    return 0;
}

void WifiManager::addListener(IWifiListener* listener)
{
    if(!listener)
        return;

    WifiListenerList::iterator it = std::find(mWifiListeners.begin(), mWifiListeners.end(), listener);
    if(listener == *it)
    {
        LOG_ERROR("WifiListener is alreay exsit !!\n");
        return;
    }
    mWifiListeners.push_front(listener);
}

void WifiManager::removeListener(IWifiListener* listener)
{
    if(!listener)
        return;

    for(WifiListenerList::iterator it = mWifiListeners.begin(); it != mWifiListeners.end(); it++)
    {
        if(listener == *it)
        {
            mWifiListeners.erase(it);
            return;
        }
    }
}

void WifiManager::notify(const WifiEvent& event)
{
    for(WifiListenerList::iterator it = mWifiListeners.begin(); it != mWifiListeners.end(); it++)
    {
        (*it)->onWifiEventReceived(event);
    }
}

int WifiManager::getFD()
{
    if (!mMonitorConn)
        return -1;

    return wpa_ctrl_get_fd(mMonitorConn);
}

bool WifiManager::onFdReadable(int fd)
{

    char buf[256];
    size_t len = sizeof(buf);
    
    int ret = wpa_ctrl_recv(mMonitorConn, buf, &len);
    if (ret != 0)
    {
        LOG_ERROR("wpa_ctrl_recv failed ! ret : %d\n", ret);
        return false;
    }

    buf[len] = 0;
    WifiEvent event(buf);
    
    if (event.getType() == WifiEvent::EVENT_TYPE_UNKNOWN)
        return true;

    notify(event);

    return true;   
}

void WifiManager::onTimerExpired(const ITimer* timer)
{
    /* TODO. IMPLEMENTS HERE */
}

void WifiManager::startSupplicant()
{
    std::string cmd;

    cmd = "/usr/sbin/wpa_supplicant ";
    cmd += "-i" WLAN_IFACE_NAME " ";
    cmd += "-D" "nl80211" " ";
    cmd += "-c" WPA_SUPPLICANT_CONF_FILE_PATH " ";
    cmd += "-P" WPA_SUPPLICANT_PID_FILE_PATH " ";
    cmd += "-m" P2P_SUPPLICANT_CONF_FILE_PATH " ";
    cmd += "-O" WPA_SUPPLICANT_CTRL_INTERFACE " ";
    cmd += "-puse_p2p_group_interface=1 ";
    cmd += "-g" WPA_SUPPLICANT_GLOBAL_IFACE " ";
    cmd += "-B";

    if (!isSupplicantRunnig())
        system(cmd.c_str());
    else
        LOG_WARN("Supplicant is already running...\n");
}

void WifiManager::stopSupplicant()
{
    // TODO CHECK THIS
    system("killall wpa_supplicant");
}

void WifiManager::init()
{
    std::string deviceName = "PRAZEN-123"; /* TODO. get from configure */
    
    sendCommand("SET country KR");
    sendCommand("IFNAME=" P2P_IFACE_NAME " SET device_type 7-0050F204-1");
    sendCommand("IFNAME=" P2P_IFACE_NAME " P2P_SET conc_pref sta");
    sendCommand("IFNAME=" P2P_IFACE_NAME " SET device_name " + deviceName);
    sendCommand("IFNAME=" P2P_IFACE_NAME " SET config_methods virtual_push_button");
    sendCommand("IFNAME=" P2P_IFACE_NAME " SET persistent_reconnect 1");
    sendCommand("IFNAME=" P2P_IFACE_NAME " P2P_SET ssid_postfix " + deviceName);
    sendCommand("IFNAME=" P2P_IFACE_NAME " SAVE_CONFIG");
}

int WifiManager::connectToSupplicant(int tryCnt)
{
    char path[2*1024];

    snprintf(path, sizeof(path), "%s", WPA_SUPPLICANT_GLOBAL_IFACE);

    for (int ii = 0; ii < tryCnt; ii++)
    {
        if(connectToSockPath(path) == 0)
            return 0;
        
        sleep(200);
    }

    if (tryCnt > 0)
        LOG_ERROR("connectToSupplicant (try : %d) is failed.\n", tryCnt);

    return -1;
}

int WifiManager::connectToSockPath(const std::string& path)
{
    mCtrlConn = wpa_ctrl_open(path.c_str());
    if (mCtrlConn == NULL)
    {
        LOG_ERROR("Unable to open conection on \"%s\" : %s\n", path, strerror(errno));
        return -1;
    }

    mMonitorConn = wpa_ctrl_open(path.c_str());
    if (mMonitorConn == NULL)
    {
        LOG_ERROR("Unable to open conection on \"%s\" : %s\n", path, strerror(errno));
        return -1;
    }

    if (wpa_ctrl_attach(mMonitorConn) != 0)
    {
        LOG_ERROR("Unable to attach monitor connection\n");
        wpa_ctrl_close(mMonitorConn);
        wpa_ctrl_close(mCtrlConn);
        mMonitorConn = mCtrlConn = NULL;
        return -1;
    }

    return 0;
}

void WifiManager::disconnectToSupplicant()
{
    // TODO IMPLMENTS HERE
}

std::string WifiManager::sendCommand(const std::string& cmd)
{
    char buf[4096];
    size_t buf_len = sizeof(buf);

    if (sendCommand(cmd.c_str(), buf, &buf_len) == 0)
    {
//LOG_Dump(LOG_LEVEL_TRACE, buf, buf_len);
        buf[buf_len] = 0;
        return buf;
    }

    return "FAIL";
}

int WifiManager::sendCommand(const char* cmd, char* reply, size_t* reply_len)
{
    int ret;

    if (mCtrlConn == NULL)
    {
        LOG_ERROR("Not connected to wpa_supplicant\n");
        return -1;
    }

    ret = wpa_ctrl_request(mCtrlConn, cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret == -2)
    {
        LOG_ERROR("Command Time out.\n");
        return -2;
    }
    else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0)
    {
        return -1;
    }
    if (strncmp(cmd, "PING", 4) == 0)
    {
        reply[*reply_len] = 0;
    }

    return 0;
}
