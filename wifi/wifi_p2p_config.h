#ifndef __WIFI_P2P_CONFIG_H_
#define __WIFI_P2P_CONFIG_H_

#include <string>

#define MAX_GROUP_OWNER_INTENT  (15)
#define MIN_GROUP_OWNER_INTENT  (0)

class WifiP2pConfig
{
public:
    /* P2P-GO-NEG-REQUEST 96:8b:c1:a4:ce:3c dev_passwd_id=4 go_intent=13 */
    WifiP2pConfig(const std::string& str);
    ~WifiP2pConfig();

    const std::string& getDeviceAddress() { return mDeviceAddress; }
    int getGroupOwnerIntent() { return mGroupOwnerIntent; }

private:
    std::string mDeviceAddress;
    
    int mGroupOwnerIntent;

private:
    static void _parse_key_value_callback(int index, const char* key, const char* value, void* userdat);

};

#endif /* __WIFI_CONFIG_H_ */
