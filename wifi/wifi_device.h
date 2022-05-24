#ifndef __WIFI_DEVICE_H_
#define __WIFI_DEVICE_H_

#include <string>

class WifiDevice
{
public:
    /* bssid / frequency / signal level / flags / ssid */
    WifiDevice(const char* str);
    ~WifiDevice();

    WifiDevice& operator=(const WifiDevice&  device);
    bool        operator==(const WifiDevice& device) const;
    bool        operator!=(const WifiDevice& device) const;

private:
    std::string mBssid;
    std::string mSsid;
    
    int mFrequency;
    int mLevel;
};

#endif /* __WIFI_DEVICE_H_ */
