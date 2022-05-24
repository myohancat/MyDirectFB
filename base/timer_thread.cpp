/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#include "timer_thread.h"

#include <errno.h>
#include "sys_time.h"
#include "log.h"


TimerThread::TimerThread()
{
    mExitTask   = false;
    mRepeat     = false;
    mIntervalMs = 0;
    mHandler    = NULL;
}

TimerThread::~TimerThread()
{
    stop();
}

void TimerThread::setHandler(ITimerHandler* handler)
{
    mHandler = handler;
}

void TimerThread::start(uint32_t msec, bool repeat)
{
    stop();

    mRepeat     = repeat;
    mStartTime  = SysTime::getTickCountMs();
    mIntervalMs = msec;
    mExitTask   = false;

    if(!Task::start())
    {
        LOG_ERROR("failed to create task\n");
    }
}

void TimerThread::stop()
{
    mExitTask = true;
    Task::stop();
}

void TimerThread::setInterval(uint32_t msec)
{
    mIntervalMs = msec;
}

void TimerThread::setRepeat(bool repeat)
{
    mRepeat = repeat;
}

uint32_t TimerThread::getInterval() const
{
    return mIntervalMs;
}

bool TimerThread::getRepeat() const
{
    return mRepeat;
}

void TimerThread::run()
{
    while(!mExitTask)
    {
        uint64_t expireTime = mStartTime + mIntervalMs;
        uint64_t timeoutMs  = expireTime - SysTime::getTickCountMs();

        sleep(timeoutMs);

        if (state() == TASK_STATE_STOPPING)
            continue;

        if (mHandler)
            mHandler->onTimerExpired(this);

        if(mRepeat == false)
            break;

        mStartTime = expireTime;
    }
}
