/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#ifndef __EVENT_LOOP_H_
#define __EVENT_LOOP_H_

#include "types.h"

#include "mutex.h"
#include "timer.h"

#include <list>

class IFdWatcher
{
public:
    virtual ~IFdWatcher() { };

    virtual int  getFD() = 0;
    virtual bool onFdReadable(int fd) = 0;
};

class EventLoop
{
public:
    static EventLoop& getInstance();

    void addFdWatcher(IFdWatcher* watcher);
    void removeFdWatcher(IFdWatcher* watcher);

    void addTimer(Timer* timer);
    void removeTimer(Timer* timer);

    bool loop();
    void wakeup();
    void terminate();

private:
    EventLoop();
    ~EventLoop();

    uint32_t runTimers();

    RecursiveMutex mFdWatcherLock;
    typedef std::list<IFdWatcher*> FdWatcherList;
    FdWatcherList mFdWatchers;

    RecursiveMutex mTimerLock;
    typedef std::list<Timer*> TimerList;
    TimerList mTimers;

    int mPipe[2];
};

#endif /* __EVENT_LOOP_H_ */
