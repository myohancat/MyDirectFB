/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#ifndef __TIMER_THREAD_H_
#define __TIMER_THREAD_H_

#include "timer.h"

#include "task.h"
#include "mutex.h"
#include "cond_var.h"

class TimerThread : public ITimer, Task
{
public:
    TimerThread();
    ~TimerThread();

    void     setHandler(ITimerHandler* handler);

    void     start(uint32_t msec, bool repeat);
    void     stop();

    void     setInterval(uint32_t msec);
    uint32_t getInterval() const;

    void     setRepeat(bool repeat);
    bool     getRepeat() const;

private:
    ITimerHandler*  mHandler;
    
    bool     mExitTask;

    bool     mRepeat;
    uint64_t mStartTime;
    int      mIntervalMs;

private:
    void     run();
};

#endif /* __TIMER_THREAD_H_ */
