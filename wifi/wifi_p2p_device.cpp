#include "wifi_p2p_device.h"

#include "key_value.h"
#include "log.h"

void WifiP2pDevice::_parse_key_value_callback(int index, const char* key, const char* value, void* userdat)
{
    WifiP2pDevice* pThis = (WifiP2pDevice*)userdat;

    if (!value)
    {
        pThis->mDeviceAddress = key;
    }
    else if(strcmp(key, "p2p_dev_addr") == 0)
    {
        pThis->mDeviceAddress = value;
    }
    else if(strcmp(key, "pri_dev_type") == 0)
    {
        pThis->mPrimaryDeviceType = value;
    }
    else if(strcmp(key, "name") == 0)
    {
        pThis->mDeviceName = value;
    }
    else if(strcmp(key, "config_methods") == 0)
    {
        if (strncmp(value, "0x", 2) == 0)
            value += 2;

        pThis->mConfigMethods = strtol(value, NULL, 16);
    }
    else if(strcmp(key, "dev_capab") == 0)
    {
        if (strncmp(value, "0x", 2) == 0)
            value += 2;

        pThis->mDeviceCapability = strtol(value, NULL, 16);
    }
    else if(strcmp(key, "group_capab") == 0)
    {
        if (strncmp(value, "0x", 2) == 0)
            value += 2;

        pThis->mGroupCapability = strtol(value, NULL, 16);
    }
    else if(strcmp(key, "wfd_dev_info") == 0)
    {
        char buf[16];
        if (strncmp(value, "0x", 2) == 0)
            value += 2;

        pThis->mWfdInfo = (WfdInfo_t*)calloc(1, sizeof(WfdInfo_t));
        
        sprintf(buf, "%c%c%c%c", value[0], value[1], value[2], value[3]);
        pThis->mWfdInfo->mDeviceInfo    = strtol(buf, NULL, 16);
        sprintf(buf, "%c%c%c%c", value[4], value[5], value[6], value[7]);
        pThis->mWfdInfo->mCtrlPort      = strtol(buf, NULL, 16);
        sprintf(buf, "%c%c%c%c", value[8], value[9], value[10], value[11]);
        pThis->mWfdInfo->mMaxThroughput = strtol(buf, NULL, 16);
    }
    else
        LOG_INFO("Unknown [%s:%s]\n", key, value);
}

WifiP2pDevice::WifiP2pDevice(const std::string& str)
              : mDeviceAddress(""),
                mDeviceName(""),
                mPrimaryDeviceType(""),
                mConfigMethods(0),
                mDeviceCapability(0),
                mGroupCapability(0),
                mWfdInfo(NULL)
{
    parse_key_value_str(str.c_str(), _parse_key_value_callback, this);

    LOG_INFO("P2P Device %s [%s]\n", mDeviceAddress.c_str(), mDeviceName.c_str());
    if (mWfdInfo)
        LOG_INFO("     (WFDInfo : deviceinfo : %04x, ctrlport : %d, maxthr : %d)\n", mWfdInfo->mDeviceInfo, mWfdInfo->mCtrlPort, mWfdInfo->mMaxThroughput);
}

WifiP2pDevice::WifiP2pDevice(const WifiP2pDevice& dev)
              : mDeviceAddress(dev.mDeviceAddress),
                mDeviceName(dev.mDeviceName),
                mPrimaryDeviceType(dev.mPrimaryDeviceType),
                mConfigMethods(dev.mConfigMethods),
                mDeviceCapability(dev.mDeviceCapability),
                mGroupCapability(dev.mGroupCapability),
                mWfdInfo(NULL)
{
    if (dev.mWfdInfo)
    {
        mWfdInfo = (WfdInfo_t*)malloc(sizeof(WfdInfo_t));
        *mWfdInfo = *dev.mWfdInfo;
    }
}

WifiP2pDevice::~WifiP2pDevice()
{
    if (mWfdInfo)
        free(mWfdInfo);    
}

WifiP2pDevice& WifiP2pDevice::operator=(const WifiP2pDevice& device)
{
    if (this != &device)
    {
        mDeviceAddress = device.mDeviceAddress;
        mDeviceName    = device.mDeviceName;
        mPrimaryDeviceType = device.mPrimaryDeviceType;
        mConfigMethods = device.mConfigMethods;
        mDeviceCapability = device.mDeviceCapability;
        mGroupCapability = device.mGroupCapability;

        if (!mWfdInfo)
            mWfdInfo = (WfdInfo_t*)malloc(sizeof(WfdInfo_t));
        *mWfdInfo = *device.mWfdInfo;
    }

    return *this;
}

bool WifiP2pDevice::operator==(const WifiP2pDevice& device) const
{
    return (mDeviceAddress == device.mDeviceAddress);
}

bool WifiP2pDevice::operator!=(const WifiP2pDevice& device) const
{
    return (mDeviceAddress != device.mDeviceAddress);
}
