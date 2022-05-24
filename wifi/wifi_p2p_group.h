#ifndef __WIFI_P2P_GROUP_H_
#define __WIFI_P2P_GROUP_H_

#include <string>

/* 
 * String pattern WFD info
 * p2p-wlan0-0 [client|GO] ssid="DIRECT-W8" freq=2437 [psk=2182b2e50e53f260d04f3c7b25ef33c965a3291b9b36b455a82d77fd82ca15bc|passphrase="fKG4jMe3"] go_dev_addr=fa:7b:7a:42:02:13 [PERSISTENT]
 */
class WifiP2pGroup 
{
public:
    WifiP2pGroup(const std::string& str);
    WifiP2pGroup(const WifiP2pGroup& group);

    ~WifiP2pGroup();

    const std::string& getInterface() { return mInterface; }
    
    const std::string& getNetworkName() { return mNetworkName; }
    
    const std::string& getOwnerAddress() { return mOwnerAddress; }

    bool isGroupOwner() { return mIsGroupOwner; }
    
private:
    std::string mInterface;

    bool        mIsGroupOwner;

    std::string mNetworkName; // ssid

    int         mFreq;

    std::string mPsk;

    std::string mPassphrase;
    
    std::string mOwnerAddress; // go_dev_addr

    bool        mIsPersitent;
private:
    static void _parse_key_value_callback(int index, const char* key, const char* value, void* userdat);
};

#endif /* __WIFI_P2P_GROUP_H_ */
