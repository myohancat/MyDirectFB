#include "input_manager.h"

#include "log.h"
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/uinput.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_PATH "/dev/input"

#include "keymap.h"

InputDevice::InputDevice(const std::string& path)
            : mFD(-1),
              mPath(path),
              mName("Unknown"),
              mVersion(-1),
              mVendor(-1),
              mProduct(-1),
              mType(INPUT_DEVICE_UNKNOWN)
{
    mFD = open(path.c_str(), O_RDWR);
    if(mFD < 0)
    {
        LOG_ERROR("cannot open file ! %s - %s\n", path.c_str(), strerror(errno));
        return;
    }
    
    loadAttrib();
    LOG_INFO("InputDevice [%s] ver : %d, vendor : %d, product : %d\n", mName.c_str(), mVersion, mVendor, mProduct);
}

InputDevice::~InputDevice()
{
    if(mFD >= 0)
        close(mFD);
}

int InputDevice::getFD()
{
    return mFD;
}

const char* InputDevice::getPath()
{
    return mPath.c_str();
}

const char* InputDevice::getName()
{
    return mName.c_str();
}

int InputDevice::getVendor()
{
    return mVendor;
}

int InputDevice::getProduct()
{
    return mProduct;
}

InputDevice_e InputDevice::getType()
{
    return mType;
}

bool InputDevice::operator==(const std::string& path) const
{
    return (mPath == path);
}

void InputDevice::loadAttrib()
{
    char buffer[80];
    struct input_id inputId;

    if (ioctl(mFD, EVIOCGNAME(sizeof(buffer) - 1), &buffer) < 1)
    {
        LOG_ERROR("Cannot getValue Input Device Name\n");
        mName = "Unknown";
        return;
    } 
    else
    {
        buffer[sizeof(buffer) - 1] = '\0';
        mName = buffer;
    }

    if(ioctl(mFD, EVIOCGID, &inputId))
    {
        LOG_ERROR("Cannot getValue Input Device ID\n");
        return;
    }
    else
    {
        mVendor  = inputId.vendor;
        mProduct = inputId.product;
        mVersion = inputId.version;
    }

    mType = get_input_device_type(mVersion, mVendor, mProduct, mName.c_str());
}

InputManager& InputManager::getInstance()
{
    static InputManager instance;

    return instance;
}

InputManager::InputManager()
             : mExitProc(false)
{
    int ret = pipe(mPipe);
    if(ret < 0)
    {
        LOG_ERROR("pipe failed (%d). %s\n", errno, strerror(errno));
        return;
    }

    mFD = inotify_init();
    mWD = inotify_add_watch(mFD, DEVICE_PATH, IN_CREATE | IN_DELETE);

#ifdef ENABLE_VIRUTAL_REPEAT_KEY
    mLastKeyCode = -1;
    mRepeatTimer.setHandler(this);
#endif

    scanInputDevice();

    EventLoop::getInstance().addFdWatcher(this);

    start();
}

InputManager::~InputManager()
{
    EventLoop::getInstance().removeFdWatcher(this);

    mExitProc = true;
    stop();

    if(mPipe[0] >= 0)
        close(mPipe[0]);
    if(mPipe[1] >= 0)
        close(mPipe[0]);

    if(mWD >= 0)
        close(mWD);
    if(mFD >= 0)
        close(mFD);
}

int InputManager::getFD()
{
    return mPipe[0];
}

#ifdef ENABLE_VIRUTAL_REPEAT_KEY
void InputManager::onTimerExpired(const ITimer* timer)
{
    int keyValue[2];

    (void)timer;

    if (mLastKeyCode == -1)
        return;

    mRepeatTimer.setInterval(100);

    /* Generate Repeat Key */
    keyValue[0] = mLastKeyCode;
    keyValue[1] = KEY_REPEATED;
    write(mPipe[1], keyValue, sizeof(keyValue));
}
#endif

bool InputManager::onFdReadable(int fd)
{
    int keyValue[2];

    if(read(fd, keyValue, sizeof(keyValue)) < 0)
    {
        LOG_ERROR("pipe read failed !\n");
        return true;
    }

    LOG_TRACE("--- [KEY] code : %d, state : %s\n", keyValue[0], (keyValue[1] == KEY_RELEASED)?"RELEASED":(keyValue[1] == KEY_PRESSED)?"PRESSED":"REPEATED");

    mKeyListenerLock.lock();
    for(KeyListenerList::iterator it = mKeyListeners.begin(); it != mKeyListeners.end(); it++)
    {
        if((*it)->onKeyReceived(keyValue[0], keyValue[1]) == true)
            break;
    }
    mKeyListenerLock.unlock();

    return true;
}

void InputManager::addKeyListener(IKeyListener* listener)
{
    Lock lock(mKeyListenerLock);

    if(!listener)
        return;

    KeyListenerList::iterator it = std::find(mKeyListeners.begin(), mKeyListeners.end(), listener);
    if(listener == *it)
    {
        LOG_ERROR("KeyListener is alreay exsit !!\n");
        return;
    }
    mKeyListeners.push_front(listener);
}

void InputManager::removeKeyListener(IKeyListener* listener)
{
    Lock lock(mKeyListenerLock);

    if(!listener)
        return;

    for(KeyListenerList::iterator it = mKeyListeners.begin(); it != mKeyListeners.end(); it++)
    {
        if(listener == *it)
        {
            mKeyListeners.erase(it);
            return;
        }
    }
}

void InputManager::setRawKeyListener(IRawKeyListener* listener)
{
    Lock lock(mRawKeyListenerLock);

    mRawKeyListener = listener;
}

void InputManager::addInputDevice(const std::string& devpath)
{
    for(InputDeviceList::iterator it = mInputDevices.begin(); it != mInputDevices.end(); it++)
    {
        InputDevice* device = *it;
        if(*device == devpath)
        {
            LOG_ERROR("device is alreay exsit : %s !!\n", devpath.c_str());
            return;
        }
    }

    InputDevice* dev = new InputDevice(devpath);
    if (dev->getFD() == -1)
    {
        LOG_ERROR("fd is invalid, skip add input device\n");
        delete dev;
        return;
    }
    
    LOG_DEBUG("---> add device : %s\n", devpath.c_str());
    mInputDevices.push_back(dev);
}

void InputManager::removeInputDevice(const std::string& devpath)
{
    for(InputDeviceList::iterator it = mInputDevices.begin(); it != mInputDevices.end(); it++)
    {
    
        InputDevice* device = *it;
        if(*device == devpath)
        {
            LOG_DEBUG("---> remove device : %s\n", devpath.c_str());
            mInputDevices.erase(it);
            return;
        }
    }
}

void InputManager::scanInputDevice()
{
    char devname[PATH_MAX];
    DIR* dir;
    struct dirent* de;

    dir = opendir(DEVICE_PATH);
    if(!dir)
    {
        LOG_ERROR("opendir failed : %s\n", DEVICE_PATH);
        return;
    }

    while((de = readdir(dir)))
    {
        struct stat st;

        if(de->d_name[0] == '.')
        { 
            if(de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2] == '\0'))
                continue;
        }

        sprintf(devname, "%s/%s", DEVICE_PATH, de->d_name);

        stat(devname, &st);
        if (S_ISDIR(st.st_mode))
            continue;

        addInputDevice(devname);
    }

    closedir(dir);
}

#define MAX(a, b)   ((a)>(b)?(a):(b))
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 16) )
void InputManager::run()
{
    fd_set sReadFds;
    struct timeval sWait;
    int    nLastFD = 0;
    int    nCnt    = 0;

    while(!mExitProc)
    {
        FD_ZERO(&sReadFds);

        FD_SET(mFD, &sReadFds); // For Watch device [ add | delete ]
        nLastFD = mFD;    

        for(InputDeviceList::iterator it = mInputDevices.begin(); it != mInputDevices.end(); it++)
        {
            int devFD = (*it)->getFD();
            FD_SET(devFD, &sReadFds);
            nLastFD = MAX(nLastFD, devFD);
        }
        nLastFD++;        

        sWait.tv_sec = 0;
        sWait.tv_usec = 100 * 1000;

        nCnt = select(nLastFD, &sReadFds, NULL, NULL, &sWait);
        if(nCnt < 0)
        {
            if(nCnt == -1 && errno != EINTR)
            {
                LOG_ERROR("select error oucced!!! errno=%d\n", errno);
                break;
            }

            continue; /* TIMEOUT */
        }
        
        if(FD_ISSET(mFD, &sReadFds))
        {
            char buffer[BUF_LEN];
            int ii = 0;
            int len = read(mFD, buffer, BUF_LEN);
            
            while( ii < len)
            {
                struct inotify_event *event = (struct inotify_event *)&buffer[ii];
                char devpath[PATH_MAX];

                if(event->len)
                {
                    if(event->mask & IN_CREATE)
                    {
                        sprintf(devpath, "%s/%s", DEVICE_PATH, event->name);
                        addInputDevice(devpath);
                    }
                    else if(event->mask & IN_DELETE)
                    {
                        sprintf(devpath, "%s/%s", DEVICE_PATH, event->name);
                        removeInputDevice(devpath);
                    }
                }

                ii += EVENT_SIZE + event->len;
            }

            continue;
        }

        for(InputDeviceList::iterator it = mInputDevices.begin(); it != mInputDevices.end(); it++)
        {
            struct input_event event;
            int res = 0;

            int devFD = (*it)->getFD();
            if(FD_ISSET(devFD, &sReadFds))
            {
                res = read(devFD, &event, sizeof(event));
                if(res >= (int)sizeof(event))
                {
                    int keyValue[2];
                    if(event.type == EV_KEY)
                    {
                        if(event.value == 0
                           || event.value == 1
                           || event.value == 2
                        )
                        {
                            int deviceType = (*it)->getType();
                            keyValue[0] = convert_key_code_from_map(deviceType, event.code);
                            keyValue[1] = event.value;

                            //LOG_TRACE("--- [RAW_KEY] code : %d, state : %d\n", keyValue[0], keyValue[1]);
                            mRawKeyListenerLock.lock();
                            if (mRawKeyListener)
                                mRawKeyListener->onRawKeyReceived(deviceType, event.code, event.value);
                            mRawKeyListenerLock.unlock();

#ifdef ENABLE_VIRUTAL_REPEAT_KEY
                            if (keyValue[1] == KEY_PRESSED)
                            {
                                mRepeatTimer.start(700, true);
                                mLastKeyCode = keyValue[0];
                            }
                            else if (keyValue[1] == KEY_RELEASED)
                            {
                                mRepeatTimer.stop();
                                mLastKeyCode = -1;
                            }
#endif
                            write(mPipe[1], keyValue, sizeof(keyValue));
                        }
                    }
                }
            }
        }
    }
}
