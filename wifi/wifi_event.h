#ifndef __WIFI_EVENT_H_
#define __WIFI_EVENT_H_

#include <string>

#include "extradata.h"

class WifiEvent
{
public:
    enum EVENT_TYPE
    {
        EVENT_TYPE_UNKNOWN = -1, 
        EVENT_TYPE_SCAN_STARTED,
        EVENT_TYPE_SCAN_RESULTS,
        EVENT_TYPE_EVENT_BSS_ADDED,
        EVENT_TYPE_EVENT_BSS_REMOVED,
        EVENT_TYPE_P2P_DEVICE_FOUND,
        EVENT_TYPE_P2P_DEVICE_LOST,
        EVENT_TYPE_P2P_PROV_DISC_PBC_REQ,
        EVENT_TYPE_P2P_GO_NEG_REQUEST,
        EVENT_TYPE_P2P_GO_NEG_SUCCESS,
        EVENT_TYPE_P2P_GO_NEG_FAILURE,
        EVENT_TYPE_P2P_INVITATION_ACCEPTED,
        EVENT_TYPE_P2P_GROUP_STARTED,
        EVENT_TYPE_P2P_GROUP_REMOVED,
        EVENT_TYPE_P2P_GROUP_FORMATION_SUCCESS,
        EVENT_TYPE_P2P_GROUP_FORMATION_FAILURE,
        EVENT_TYPE_P2P_FAILED_TO_ADD_GROUP,
        EVENT_TYPE_P2P_FIND_STOPPED,
        EVENT_TYPE_EVENT_CONNECTED,
        EVENT_TYPE_EVENT_DISCONNECTED,
        EVENT_TYPE_AP_ENABLED,
        EVENT_TYPE_AP_DISABLED,
        EVENT_TYPE_AP_STA_CONNECTED,
        EVENT_TYPE_AP_STA_DISCONNECTED,
        EVENT_TYPE_TRY_TO_ASSOCIATE,
        EVENT_TYPE_P2P_INVITATION_RESULT,
        EVENT_TYPE_EVENT_SSID_TEMP_DISABLED,
        EVENT_TYPE_EVENT_NETWORK_NOT_FOUND,
        EVENT_TYPE_WPS_TIMEOUT,
        EVENT_TYPE_WPS_PSK_KEY,
    };

    WifiEvent(char* wpaEventStr);
    ~WifiEvent();

    int getType() const;
    const std::string& getInterface();

    void* getExtraData() const;

private:
    int         mType;
    std::string mInterface;
    
    ExtraData*  mExtraData;

private:
    void setExtraData(ExtraData::DataType_e type, void* obj);

    void parseWpaEvent(char* str);
    void parseEventMsg(char* str);
    void parseRequestMsg(char* str);
    void parseP2pMsg(char* str);
    void parseOtherMsg(char* str);
};

#endif /* __WIFI_EVENT_H_ */
