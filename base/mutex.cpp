/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#include "mutex.h"

Mutex::Mutex(bool isRecusive)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);

    if (isRecusive)
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&mId, &attr);
    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mId);
}

void Mutex::lock()
{
    pthread_mutex_lock(&mId);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&mId);
}

