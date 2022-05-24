/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#include "task.h"

#include <unistd.h>
#include "log.h"

Task::Task()
     : mId(-1),
       mState(TASK_STATE_INIT)
{
}

Task::~Task()
{
}

bool Task::start()
{
    Lock lock(mMutex);

    if (mState != TASK_STATE_INIT && mState != TASK_STATE_STOPPED)
    {
        LOG_WARN("task is already running\n");
        return false;
    }
    
    if (mState == TASK_STATE_STOPPED)
    {
        mMutex.unlock();
        stop();
        mMutex.lock();
    }

    if (!onPreStart())
    {
        LOG_ERROR("onPreStart() failed !\n");
        return false;
    }

    if (pthread_create (&mId, NULL, _task_proc_priv, this) != 0)
    {
        LOG_ERROR("pthread create failed !\n");
        return false;
    }

    onPostStart();

    mState = TASK_STATE_RUNNING;

    return true;
}

void Task::stop()
{
    Lock lock(mMutex);

    if (mState != TASK_STATE_RUNNING && mState != TASK_STATE_STOPPED)
        return;

    onPreStop();

    mState = TASK_STATE_STOPPING;
    wakeup();

    if (mId != (pthread_t)-1)
    {
        mMutex.unlock();
        pthread_join(mId, NULL);
        mMutex.lock();
        mId = -1;
    }

    mState = TASK_STATE_INIT;
    onPostStop();
}

void Task::sleep(int msec)
{
    Lock lock(mMutex);

    if (mState == TASK_STATE_STOPPING)
        return;

    if (mId == pthread_self())
        mCond.wait(mMutex, msec);
    else
        usleep(msec * 1000);
}

TaskState Task::state()
{
    return mState;
}

void Task::wakeup()
{
    mCond.signal();
}

void* Task::_task_proc_priv(void* param)
{
    Task* pThis = static_cast<Task*>(param);

    pThis->run();

    pThis->mMutex.lock();
    pThis->mState = TASK_STATE_STOPPED;
    pThis->mMutex.unlock();

    return NULL;
}
