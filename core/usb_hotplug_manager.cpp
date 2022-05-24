#include "usb_hotplug_manager.h"

#include <stdlib.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "log.h"



USB_HotplugManager& USB_HotplugManager::getInstance()
{
    static USB_HotplugManager _instance;

    return _instance;
}

USB_HotplugManager::USB_HotplugManager()
{
    struct sockaddr_nl addr;

    mSock = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if(mSock < 0)
    {
        LOG_ERROR("socket error (%d) - %s\n", errno, strerror(errno));
        exit(-2);
    }

    memset(&addr, 0x00, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK;

    if(bind(mSock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        LOG_ERROR("bind failed (%d) - %s\n", errno, strerror(errno));
        exit(-2);
    }

    EventLoop::getInstance().addFdWatcher(this);
}

USB_HotplugManager::~USB_HotplugManager()
{
    EventLoop::getInstance().removeFdWatcher(this);
    
    if(mSock >= 0)
        close(mSock);
}

int USB_HotplugManager::getFD()
{
    return mSock;
}

bool USB_HotplugManager::onFdReadable(int fd)
{
    USB_Hotplug_Event_e eEvent;

    int    ret = 0;
    char   buf[4096];
    char*  action;
    char*  devpath;
    struct sockaddr_nl nladdr;
    struct iovec iov = 
    { 
        .iov_base = buf,
        .iov_len  = sizeof(buf),
    };

    struct msghdr msg = 
    {
        .msg_name    = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov     = &iov,
        .msg_iovlen  = 1,
        .msg_control = NULL,
        .msg_controllen = 0,
        .msg_flags = 0,
    };

    ret = recvmsg(fd, &msg, 0);
    if(ret < 0)
        return true;

    if(ret == 0)
    {
        LOG_ERROR("EOF on netlink\n");
        return false;
    }

    if(msg.msg_namelen != sizeof(nladdr))
    {
        LOG_ERROR("Wrong address length\n");
        return true;
    }

    if(iov.iov_len < ((size_t)ret) || (msg.msg_flags & MSG_TRUNC))
    {
        LOG_ERROR("Malformatted or truncated message, skipping\n");
        return true;
    }

    buf[ret] = 0;
//    LOG_TRACE("==> %s\n", buf);
    action = buf;
    devpath = strchr(buf, '@');
    if(devpath == NULL)
        return true;

    *devpath = 0;
    devpath++;

    if(strcmp(action, "add") == 0)
        eEvent = USB_HOTPLUG_EVENT_ADD;
    else if(strcmp(action, "change") == 0)
        eEvent = USB_HOTPLUG_EVENT_CHANGE;
    else if(strcmp(action, "remove") == 0)
        eEvent = USB_HOTPLUG_EVENT_REMOVE;
    else
        eEvent = USB_HOTPLUG_EVENT_UNKNOWN;
        
    notifyHotplugEvent(eEvent, devpath);

    return true;
}

void USB_HotplugManager::notifyHotplugEvent(USB_Hotplug_Event_e eEvent, const char* devPath)
{
    int count = mListeners.size();
    int ii;

    for(ii = 0; ii < count; ii++)
    {
        mListeners[ii]->onHotplugChanged(eEvent, devPath);
    }
}

void USB_HotplugManager::addListener(IUSB_HotplugListener* listener)
{
    mListeners.push_back(listener);
}

void USB_HotplugManager::removeListener(IUSB_HotplugListener* listener)
{
    int count = mListeners.size();
    int ii;

    for(ii = 0; ii < count; ii++)
    {
        if(mListeners[ii] == listener)
            break;
    }

    if(ii < count)
        mListeners.erase(mListeners.begin() + ii);
}

