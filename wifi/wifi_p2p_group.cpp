#include "wifi_p2p_group.h"

#include <string.h>

#include "log.h"
#include "key_value.h"

void WifiP2pGroup::_parse_key_value_callback(int index, const char* key, const char* value, void* userdat)
{
    WifiP2pGroup* pThis = (WifiP2pGroup*)userdat;

    if (!value)
    {
        if (index == 0)
            pThis->mInterface = key;
        else if (index == 1)
            pThis->mIsGroupOwner = (strcmp(key, "GO") == 0);
        else
            pThis->mIsPersitent  = (strcmp(key, "[PERSISTENT") == 0);
    }
    else if(strcmp(key, "ssid") == 0)
    {
        pThis->mNetworkName = value;
    }
    else if (strcmp(key, "freq") == 0)
    {
        pThis->mFreq = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "psk") == 0)
    {
        pThis->mPsk = value;
    }
    else if (strcmp(key, "passphrase") == 0)
    {
        pThis->mPassphrase = value;
    }
    else if (strcmp(key, "go_dev_addr") == 0)
    {
        pThis->mOwnerAddress = value;
    }
    else
        LOG_INFO("Unkown [%s:%s]\n", key, value);
}

WifiP2pGroup::WifiP2pGroup(const std::string& str)
             : mInterface(""),
               mIsGroupOwner(false),
               mNetworkName(""),
               mFreq(0),
               mPsk(""),
               mPassphrase(""),
               mOwnerAddress(""),
               mIsPersitent(false)
{
    parse_key_value_str(str.c_str(), _parse_key_value_callback, this);
}

WifiP2pGroup::WifiP2pGroup(const WifiP2pGroup& group)
             : mInterface(group.mInterface),
               mIsGroupOwner(group.mIsGroupOwner),
               mNetworkName(group.mNetworkName),
               mFreq(group.mFreq),
               mPsk(group.mPsk),
               mPassphrase(group.mPassphrase),
               mOwnerAddress(group.mOwnerAddress),
               mIsPersitent(group.mIsPersitent)
{
}

WifiP2pGroup::~WifiP2pGroup()
{
}
