#include "wifi_device.h"

#include <string.h>

#include "log.h"

WifiDevice::WifiDevice(const char* str)
           : mBssid(""),
             mSsid(""),
             mFrequency(0),
             mLevel(0)
{
    char* tok,* saveptr;
    int   pos;
    char buf[1024];

    strncpy(buf, str, sizeof(buf));
    buf[sizeof(buf) -1] = 0;

    for (pos = 0, tok = strtok_r(buf, "\t", &saveptr); tok; tok = strtok_r(NULL, "\t", &saveptr), pos++)
    {
        switch(pos)
        {
            case 0: /* bssid */
                mBssid = tok;
                break;
            case 1: /* frequency */
                mFrequency = strtol(tok, NULL, 10);
                break;
            case 2: /* signal level */
                mLevel = strtol(tok, NULL, 10);
                break;
            case 3: /* flags */
                break;
            case 4: /* ssid */
                mSsid = tok;
                break;
            default:
                LOG_ERROR("exceed token position : %d (%s)\n", pos, tok);
                break;
        }
    }

    // LOG_TRACE("BSSID : %s, Freq : %d, Level : %d, SSID : %s\n", mBssid.c_str(), mFrequency, mLevel, mSsid.c_str());
}

WifiDevice::~WifiDevice()
{
}


WifiDevice& WifiDevice::operator=(const WifiDevice& device)
{
    if (this != &device)
    {
        mBssid = device.mBssid;
        mSsid  = device.mSsid;
        mFrequency = device.mFrequency;
        mLevel = device.mLevel;
    }

    return *this;
}

bool WifiDevice::operator==(const WifiDevice& device) const
{
    return mBssid == device.mBssid;
}

bool WifiDevice::operator!=(const WifiDevice& device) const
{
    return mBssid != device.mBssid;
}
