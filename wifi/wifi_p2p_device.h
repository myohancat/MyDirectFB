#ifndef __WIFI_P2P_DEVICE_H_
#define __WIFI_P2P_DEVICE_H_

#include <string>

typedef struct WfdInfo_s
{
    uint16_t mReserv; /* not used */
    uint16_t mDeviceInfo;
    uint16_t mCtrlPort;
    uint16_t mMaxThroughput;
} WfdInfo_t;

/* 
 * String pattern WFD info
 * 96:8b:c1:a4:ce:3c p2p_dev_addr=96:8b:c1:a4:ce:3c pri_dev_type=10-0050F204-5 name='[Phone] Galaxy Note8' config_methods=0x188 dev_capab=0x25 group_capab=0x0
 */
class WifiP2pDevice
{
public:
    WifiP2pDevice(const std::string& str);
    WifiP2pDevice(const WifiP2pDevice& dev);

    ~WifiP2pDevice();

    WfdInfo_t* getWfdInfo() const { return mWfdInfo; }

    const std::string& getDeviceAddress() { return mDeviceAddress; }
    const std::string& getDeviceName() { return mDeviceName; }
    const std::string& getPrimaryDeviceType() { return mPrimaryDeviceType; }

    int getConfigMethods() { return mConfigMethods; }
    int getDeviceCapability() { return mDeviceCapability; }
    int getGroupCapability() { return mGroupCapability; }

    WifiP2pDevice& operator=(const WifiP2pDevice&  device);
    bool           operator==(const WifiP2pDevice& device) const;
    bool           operator!=(const WifiP2pDevice& device) const;

private:
    std::string mDeviceAddress;
    std::string mDeviceName;
    std::string mPrimaryDeviceType;

#define WPS_CONFIG_DISPLAY                     (0x0008)
#define WPS_CONFIG_PUSHBUTTON                  (0x0080)
#define WPS_CONFIG_KEYPAD                      (0x0100)
    int mConfigMethods; /* WPS Config Methods */

#define DEVICE_CAPAB_SERVICE_DISCOVERY         (0x01)
#define DEVICE_CAPAB_CLIENT_DISCOVERABILITY    (0x02)
#define DEVICE_CAPAB_CONCURRENT_OPER           (0x04)
#define DEVICE_CAPAB_INFRA_MANAGED             (0x08)
#define DEVICE_CAPAB_DEVICE_LIMIT              (0x10)
#define DEVICE_CAPAB_INVITATION_PROCEDURE      (0x20)
    int mDeviceCapability;

#define GROUP_CAPAB_GROUP_OWNER                (0x01) 
#define GROUP_CAPAB_PERSISTENT_GROUP           (0x02)
#define GROUP_CAPAB_GROUP_LIMIT                (0x04)
#define GROUP_CAPAB_INTRA_BSS_DIST             (0x08)
#define GROUP_CAPAB_CROSS_CONN                 (0x10)
#define GROUP_CAPAB_PERSISTENT_RECONN          (0x20)
#define GROUP_CAPAB_GROUP_FORMATION            (0x40)
    int mGroupCapability;
    
    WfdInfo_t* mWfdInfo;

private:
    static void _parse_key_value_callback(int index, const char* key, const char* value, void* userdat);
};

#endif /* __WIFI_DEVICE_H_ */
