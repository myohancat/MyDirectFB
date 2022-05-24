/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#ifndef __MUTEX_H_
#define __MUTEX_H_

#include <pthread.h>

class Mutex
{
friend class CondVar;

public:
    Mutex(bool isRecusive = false);
    ~Mutex();

    void lock();
    void unlock();

private:
    Mutex(const Mutex&) = delete;
    Mutex& operator= (const Mutex&) = delete;

private:
    pthread_mutex_t mId;    
        
};

class RecursiveMutex : public Mutex
{
public:
    RecursiveMutex() : Mutex(true) { }

};

class Lock
{
public:
    inline Lock(Mutex& mutex) : mMutex(mutex)  { mMutex.lock(); }
    inline Lock(Mutex* mutex) : mMutex(*mutex) { mMutex.lock(); }
    inline ~Lock() { mMutex.unlock(); }

private:
    Mutex& mMutex;
};

#endif /* __MUTEX_H_ */
