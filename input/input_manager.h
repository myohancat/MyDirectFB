#ifndef __INPUT_MANAGER_H_
#define __INPUT_MANAGER_H_

#include "types.h"

#include "keycode.h"

#include "event_loop.h"
#include "timer_thread.h"

#include "task.h"
#include "mutex.h"

#include <string>
#include <list>

#define ENABLE_VIRUTAL_REPEAT_KEY

class IKeyListener
{
public:
    virtual ~IKeyListener() { }
    
    virtual bool onKeyReceived(int keyCode, int state) = 0;
};

class IRawKeyListener
{
public:
    virtual ~IRawKeyListener() { }

    virtual void onRawKeyReceived(int device, int code, int value) = 0;
};

class InputDevice
{
public:
    InputDevice(const std::string& path);
    ~InputDevice();

    int getFD();

    const char*   getPath();
    const char*   getName();
    int           getVendor();
    int           getProduct();
    InputDevice_e getType();

    bool operator==(const std::string& path) const;

private:
    int mFD;

    std::string   mPath;
    std::string   mName;
    int           mVersion;
    int           mVendor;
    int           mProduct;
    InputDevice_e mType;

private:
    void loadAttrib();
};

class InputManager : public Task, IFdWatcher
#ifdef ENABLE_VIRUTAL_REPEAT_KEY
                          , ITimerHandler
#endif
{
public:
    static InputManager& getInstance();

    void addKeyListener(IKeyListener* listener);
    void removeKeyListener(IKeyListener* listener);

    void setRawKeyListener(IRawKeyListener* listener);

private:
    InputManager();
    ~InputManager();

    int mPipe[2];

    int mFD; /* inotify_init() */
    int mWD; /* inotify_add_watch() */

    bool      mExitProc;

#ifdef ENABLE_VIRUTAL_REPEAT_KEY
    int         mLastKeyCode;
    TimerThread mRepeatTimer;
#endif

    RecursiveMutex mKeyListenerLock;
    typedef std::list<IKeyListener*> KeyListenerList;
    KeyListenerList mKeyListeners;

    RecursiveMutex mRawKeyListenerLock;
    IRawKeyListener* mRawKeyListener;

    /* Input Device */
    typedef std::list<InputDevice*> InputDeviceList;
    InputDeviceList     mInputDevices;

private:
    int getFD();
    bool onFdReadable(int fd);

#ifdef ENABLE_VIRUTAL_REPEAT_KEY
    void onTimerExpired(const ITimer* timer);
#endif

    void scanInputDevice();
    void addInputDevice(const std::string& devpath);
    void removeInputDevice(const std::string& devpath);

    bool startKeyEventTask();
    bool stopKeyEventTask();

    void run();
};

#endif /* __INPUT_MANAGER_H_ */
