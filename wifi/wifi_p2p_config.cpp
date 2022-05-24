#include "wifi_p2p_config.h"

#include <string.h>

#include "log.h"
#include "key_value.h"

/* P2P-GO-NEG-REQUEST 96:8b:c1:a4:ce:3c dev_passwd_id=4 go_intent=13 */
void WifiP2pConfig::_parse_key_value_callback(int index, const char* key, const char* value, void* userdat)
{
    WifiP2pConfig* pThis = (WifiP2pConfig*)userdat;

    if (!value)
    {
        pThis->mDeviceAddress = key;
    }
    else if(strcmp(key, "go_intent") == 0)
    {
        pThis->mGroupOwnerIntent = strtol(value, NULL, 10);
        LOG_DEBUG("-- go_intent : %d\n", pThis->mGroupOwnerIntent);
    }
    else
        LOG_INFO("Unknown [%s:%s]\n", key, value);
}

WifiP2pConfig::WifiP2pConfig(const std::string& str)
              : mDeviceAddress(""),
                mGroupOwnerIntent(-1)
{
    parse_key_value_str(str.c_str(), _parse_key_value_callback, this);
}

WifiP2pConfig::~WifiP2pConfig()
{
}
