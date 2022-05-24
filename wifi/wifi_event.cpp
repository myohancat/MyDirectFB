#include "wifi_event.h"

#include <string.h>

#include "wifi_device.h"
#include "wifi_p2p_device.h"
#include "wifi_p2p_group.h"
#include "wifi_p2p_config.h"

#include "trim.h"
#include "log.h"

static const char* EVENT_PREFIX_STR = "CTRL-EVENT-";
static int         EVENT_PREFIX_LEN = strlen(EVENT_PREFIX_STR);

static const char* REQUEST_PREFIX_STR = "CTRL-REQ-";
static int         REQUEST_PREFIX_LEN = strlen(REQUEST_PREFIX_STR);

static const char* P2P_PREFIX_STR = "P2P-";
static int         P2P_PREFIX_LEN = strlen(P2P_PREFIX_STR);

WifiEvent::WifiEvent(char* wpaEventStr)
          : mType (EVENT_TYPE_UNKNOWN),
            mInterface(""),
            mExtraData(NULL)
{
    parseWpaEvent(wpaEventStr);
}

WifiEvent::~WifiEvent()
{
    if (mExtraData)
        delete mExtraData;
}

int WifiEvent::getType() const
{
    return mType;
}

const std::string& WifiEvent::getInterface()
{
    return mInterface;
}

void* WifiEvent::getExtraData() const
{
    return mExtraData->object();
}

void WifiEvent::parseEventMsg(char* str)
{
    if (strncmp(str, "SCAN-STARTED", 12) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_SCAN_STARTED\n", mInterface.c_str());
        mType = EVENT_TYPE_SCAN_STARTED;
    }
    else if (strncmp(str, "BSS-ADDED", 9) == 0)
    {
        char* start, *end;
        int networkId = -1;
        str += 9;

        mType = EVENT_TYPE_EVENT_BSS_ADDED;
        if ((start = strchr(str, ' ')) != NULL)
        {
            start ++;
            if ((end = strchr(str, ' ')) != NULL)
            {
                *end = 0;
                networkId = atoi(start);
                str = end + 1;
            }
        }
        LOG_INFO("[%s] EVENT_TYPE_EVENT_BSS_ADDED [%d, %s]\n", mInterface.c_str(), networkId, str);
    }
    else if (strncmp(str, "BSS-REMOVED", 11) == 0)
    {
        char* start, *end;
        int networkId = -1;
        str += 11;

        mType = EVENT_TYPE_EVENT_BSS_REMOVED;
        if ((start = strchr(str, ' ')) != NULL)
        {
            start ++;
            if ((end = strchr(start, ' ')) != NULL)
            {
                *end = 0;
                networkId = atoi(start);
                str = end + 1;
            }
        }
        LOG_INFO("[%s] EVENT_TYPE_EVENT_BSS_REMOVED [%d, %s]\n", mInterface.c_str(), networkId, str);
    }
    else if (strncmp(str, "SCAN-RESULTS", 12) == 0)
    {
        mType = EVENT_TYPE_SCAN_RESULTS;
        LOG_INFO("[%s] EVENT_TYPE_SCAN_RESULTS\n", mInterface.c_str());
    }
    
    else if (strncmp(str, "NETWORK-NOT-FOUND", 17) == 0)
    {
        mType = EVENT_TYPE_EVENT_NETWORK_NOT_FOUND;
        LOG_INFO("[%s] EVENT_TYPE_EVENT_NETWORK_NOT_FOUND\n", mInterface.c_str());
    }
    /* CTRL-EVENT-CONNECTED - Connection to 00:1e:58:ec:d5:6d completed (reauth) [id=1 id_str=] */
    else if (strncmp(str, "CONNECTED", 9) == 0)
    {
        str += 9;
            
        LOG_INFO("[%s] EVENT_TYPE_EVENT_CONNECTED\n", mInterface.c_str());
    }
    /* CTRL-EVENT-DISCONNECTED - bssid=ac:22:0b:24:70:74 reason=3 locally_generated=1 */
    else if (strncmp(str, "DISCONNECTED", 12) == 0)
    {
        str += 12;

        LOG_INFO("[%s] EVENT_TYPE_EVENT_DISCONNECTED\n", mInterface.c_str());
    }
    else
        LOG_INFO("[%s][CTRL-EVENT] \"%s\"\n", mInterface.c_str(), str);
}

void WifiEvent::parseRequestMsg(char* str)
{
    LOG_INFO("[%s][CTRL-REQ] \"%s\"\n", mInterface.c_str(), str);
}

void WifiEvent::parseP2pMsg(char* str)
{
    /* P2P-DEVICE-FOUND fa:7b:7a:42:02:13 p2p_dev_addr=fa:7b:7a:42:02:13 pri_dev_type=1-0050F204-1 name='p2p-TEST1' config_methods=0x188 dev_capab=0x27 group_capab=0x0 */
    if (strncmp(str, "DEVICE-FOUND", 12) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_DEVICE_FOUND\n", mInterface.c_str());
        str += 12;
        mType = EVENT_TYPE_P2P_DEVICE_FOUND;
        
        WifiP2pDevice* dev = new WifiP2pDevice(str);
        setExtraData(ExtraData::TYPE_WIFI_P2P_DEVICE, dev);
    }
    /* P2P-DEVICE-LOST p2p_dev_addr=42:fc:89:e1:e2:27 */
    else if (strncmp(str, "DEVICE-LOST", 11) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_DEVICE_LOST\n", mInterface.c_str());
        str += 11;
        mType = EVENT_TYPE_P2P_DEVICE_LOST;

        WifiP2pDevice* dev = new WifiP2pDevice(str);
        setExtraData(ExtraData::TYPE_WIFI_P2P_DEVICE, dev);
    }
    /* P2P-PROV-DISC-PBC-REQ 42:fc:89:e1:e2:27 p2p_dev_addr=42:fc:89:e1:e2:27 pri_dev_type=1-0050F204-1 name='p2p-TEST2' config_methods=0x188 dev_capab=0x27 group_capab=0x0 */
    else if (strncmp(str, "PROV-DISC-PBC-REQ", 17) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_PROV_DISC_PBC_REQ\n", mInterface.c_str());
        str += 17;
        mType = EVENT_TYPE_P2P_PROV_DISC_PBC_REQ;

        WifiP2pDevice* dev = new WifiP2pDevice(str);
        setExtraData(ExtraData::TYPE_WIFI_P2P_DEVICE, dev);
    }
    /* P2P-INVITATION-ACCEPTED sa=9e:78:9c:9f:a9:65 persistent=0 */
    else if (strncmp(str, "INVITATION-ACCEPTED", 19) == 0)
    {
        char* start, *end;
        LOG_INFO("[%s] EVENT_TYPE_P2P_INVITATION_ACCEPTED\n", mInterface.c_str());
        str += 19;
        mType = EVENT_TYPE_P2P_INVITATION_ACCEPTED;
        
        start = strstr(str, "sa=");
        if (start)
        {
            start += 3;
            end = strchr(start, ' ');
            if (end) *end = 0;

            setExtraData(ExtraData::TYPE_STRING, new std::string(rtrim(start)));
        }
    }
    /* P2P-GO-NEG-REQUEST 42:fc:89:a8:96:09 dev_passwd_id=4 */
    else if (strncmp(str, "GO-NEG-REQUEST", 14) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GO_NEG_REQUEST\n", mInterface.c_str());
        str += 14;
        mType = EVENT_TYPE_P2P_GO_NEG_REQUEST;

        WifiP2pConfig* config = new WifiP2pConfig(str);
        setExtraData(ExtraData::TYPE_WIFI_P2P_CONFIG, config);
    }
    /* P2P-GO-NEG-SUCCESS role=GO freq=5200 ht40=0 peer_dev=c6:e7:72:49:c2:39 peer_iface=c6:e7:72:49:c2:39 wps_method=PBC */
    else if (strncmp(str, "GO-NEG-SUCCESS", 14) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GO_NEG_SUCCESS\n", mInterface.c_str());
        str += 14;
        mType = EVENT_TYPE_P2P_GO_NEG_SUCCESS;
        /* TODO IMPLEMENTS HERE */
    }
    /* P2P-GO-NEG-FAILURE status=x */
    else if (strncmp(str, "GO-NEG-FAILURE", 14) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GO_NEG_FAILURE\n", mInterface.c_str());
        str += 14;
        mType = EVENT_TYPE_P2P_GO_NEG_FAILURE;
        /* TODO IMPLEMENTS HERE */
    }
    /* P2P-GROUP-STARTED p2p-wlan0-0 client ssid="DIRECT-l4-SM-N950N" freq=5745 psk=d0fa092cfa319e9122324c98b76c75fd036883a421767df5e62227a766be005d go_dev_addr=96:8b:c1:a4:ce:3c [PERSISTENT] */
    else if (strncmp(str, "GROUP-STARTED", 13) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GROUP_STARTED\n", mInterface.c_str());
        str += 13;
        mType = EVENT_TYPE_P2P_GROUP_STARTED;

        WifiP2pGroup* group = new WifiP2pGroup(str);
        setExtraData(ExtraData::TYPE_WIFI_P2P_GROUP, group);
    }
    /* P2P-GROUP-REMOVED p2p-wlan0-0 client reason=GO_ENDING_SESSION */
    else if (strncmp(str, "GROUP-REMOVED", 13) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GROUP_REMOVED\n", mInterface.c_str());
        str += 13;
        mType = EVENT_TYPE_P2P_GROUP_REMOVED;
    }
    /* P2P-GROUP-FORMATION-SUCCESS */
    else if (strncmp(str, "GROUP-FORMATION-SUCCESS", 23) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GROUP_FORMATION_SUCCESS\n", mInterface.c_str());
        str += 23;
        mType = EVENT_TYPE_P2P_GROUP_FORMATION_SUCCESS;
    }
    
    else if (strncmp(str, "GROUP-FORMATION-FAILURE", 23) == 0)
    {
        LOG_INFO("[%s] EVENT_TYPE_P2P_GROUP_FORMATION_FAILURE\n", mInterface.c_str());
        str += 23;
        mType = EVENT_TYPE_P2P_GROUP_FORMATION_FAILURE;
    }
    else
        LOG_INFO("[%s] [P2P] %s\n", mInterface.c_str(), str);
}

void WifiEvent::parseOtherMsg(char* str)
{
    LOG_INFO("[OTHER] \"%s\"\n", str);
    if (strncmp(str, "Trying to associate with", 24) == 0)
    {
        char* start, *end;
        int networkId = -1;
        str += 24;
        mType = EVENT_TYPE_TRY_TO_ASSOCIATE;
        LOG_INFO("[%s] EVENT_TYPE_TRY_TO_ASSOCIATE\n", mInterface.c_str());

#if 0
        if ((start = strchr(str, ' ')) != NULL)
        {
            str ++;
            if ((end = strchr(str, ' ')) != NULL)
            {
                *end = 0;
                LOG_INFO("     BSSID : %s\n", str);
            }
        }
#endif
    }
    else if (strncmp(str, "WPS-TIMEOUT", 11) == 0)
    {
        mType = EVENT_TYPE_WPS_TIMEOUT;
        LOG_INFO("[%s] EVENT_TYPE_WPS_TIMEOUT\n", mInterface.c_str());
    }
}

void WifiEvent::parseWpaEvent(char* str)
{
    char* start;
    char* end;

    if (str == NULL)
        return;
    
    /* Parse Interface */
    start = strstr(str, "IFNAME=");
    if (start)
    {
        start += 7;
        end = strchr(start, ' ');
        if (end)
        {
            *end++ = 0;
            mInterface = start;
            str = end;
        }
    }
    else
    {
        mInterface = "";
    }

    /* Parse <?> */
    start = strchr(str, '<');
    if (start)
    {
        start++;
        end = strchr(start, '>');
        if (end)
        {
            *end++;
            // TODO IS NEED TO PARSE
            str = end;
        } 
    }

//LOG_TRACE("%s\n", str);
    if (strncmp(str, EVENT_PREFIX_STR, EVENT_PREFIX_LEN) == 0)
    {
        str = str + EVENT_PREFIX_LEN;
        parseEventMsg(str);
    }
    else if (strncmp(str, REQUEST_PREFIX_STR, REQUEST_PREFIX_LEN) == 0)
    {
        str = str + REQUEST_PREFIX_LEN;
        parseRequestMsg(str);
    }
    else if (strncmp(str, P2P_PREFIX_STR, P2P_PREFIX_LEN) == 0)
    {
        str = str + P2P_PREFIX_LEN;
        parseP2pMsg(str);
    }
    else
        parseOtherMsg(str);
}

void WifiEvent::setExtraData(ExtraData::DataType_e type, void* obj)
{
    if(!mExtraData)
        delete(mExtraData);

    mExtraData = new ExtraData(type, obj);
}
