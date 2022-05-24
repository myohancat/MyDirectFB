#ifndef __USB_HOTPLUG_MANAGER_H_
#define __USB_HOTPLUG_MANAGER_H_

#include "event_loop.h"
#include <vector>

typedef enum
{
    USB_HOTPLUG_EVENT_ADD,
    USB_HOTPLUG_EVENT_CHANGE,
    USB_HOTPLUG_EVENT_REMOVE,

    USB_HOTPLUG_EVENT_UNKNOWN /* DON'T MODIFY THIS */

}USB_Hotplug_Event_e;


class IUSB_HotplugListener
{
public:
    virtual ~IUSB_HotplugListener() { }

    virtual void onHotplugChanged(USB_Hotplug_Event_e event, const char* devPath) = 0;
};


class USB_HotplugManager : public IFdWatcher
{
public:
    static USB_HotplugManager& getInstance();

    void addListener(IUSB_HotplugListener* listener);
    void removeListener(IUSB_HotplugListener* listener);

private:
    int mSock;

    std::vector<IUSB_HotplugListener*> mListeners;

private:
    USB_HotplugManager();
    ~USB_HotplugManager();

    int  getFD();
    bool onFdReadable(int fd);
    
    void notifyHotplugEvent(USB_Hotplug_Event_e eEvent, const char* devPath);

};


#endif /* __USB_HOTPLUG_MANAGER_H_ */
