#ifndef __SIMPLE_RTP_PLAYER_H_
#define __SIMPLE_RTP_PLAYER_H_

#include "mutex.h"
#include "task.h"

class SimpleRtpPlayer : public Task
{
public:
    SimpleRtpPlayer();
    ~SimpleRtpPlayer();

    bool start(int port);
    void stop();

private:
    int mPORT;
    int mPID;

    bool mExitTask;

    Mutex mPidLock;

private:
    void run();
};

#endif /* __SIMPLE_RTP_PLAYER_H_ */
