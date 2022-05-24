/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#ifndef __TASK_H_
#define __TASK_H_

#include "mutex.h"
#include "cond_var.h"

#include <pthread.h>

typedef enum
{
    TASK_STATE_INIT,
    TASK_STATE_RUNNING,
    TASK_STATE_STOPPING,
    TASK_STATE_STOPPED,
} TaskState;

class Task
{
public:
    Task();
    virtual ~Task();

    bool start();
    void stop();

    void sleep(int msec);
    void wakeup();

    TaskState state();

    virtual void run() = 0;
    
    virtual bool onPreStart() { return true; }
    virtual void onPostStart() { }

    virtual void onPreStop() { }
    virtual void onPostStop() { }

private:
    static void* _task_proc_priv(void* param);

private:
    pthread_t  mId;

    Mutex      mMutex;
    CondVar    mCond;
    TaskState  mState;
};

#endif /* __TASK_H_ */
