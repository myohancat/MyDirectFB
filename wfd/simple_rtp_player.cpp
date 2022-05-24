#include "simple_rtp_player.h"

#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include "log.h"
#include "util.h"

SimpleRtpPlayer::SimpleRtpPlayer()
                : mPORT(-1),
                  mPID(-1),
                  mExitTask(1)
{

}

SimpleRtpPlayer::~SimpleRtpPlayer()
{
    stop();
}

bool SimpleRtpPlayer::start(int port)
{
    mPORT    = port;
    mPID     = -1;
    mExitTask = 0;

    return Task::start();
}

void SimpleRtpPlayer::stop()
{
    /* TODO Race condition */
__TRACE_FUNC__;
    mPidLock.lock();
    mExitTask = 1;
    if (mPID >= 0)
        kill(mPID, SIGKILL);
    mPidLock.unlock();

    Task::stop();
}

void SimpleRtpPlayer::run()
{
    char cmd[1024];
    char buf[1024];
    int  ret;
    FILE* fp;
    sprintf(cmd, "omxplayer rtp://127.0.0.1:%d --live --threshold 0 --aidx -1 --no-keys", mPORT);

    LOG_DEBUG(">> CMD : %s\n", cmd);
    LOG_INFO("---- Start RTP Player Proc -----\n");    
    mPidLock.lock();
    if (mExitTask)
    {
        mPidLock.unlock();
        goto EXIT;
    }
    fp = popen2(cmd, "r", &mPID);
    if (fp == NULL)
    {
        LOG_ERROR("popen2 failed, fp is NULL\n");
        mPID = -1;
        mPidLock.unlock();
        goto EXIT;
    }
    mPidLock.unlock();

    while(fgets(buf, sizeof(buf), fp))
    {
        LOG_DEBUG("----- recved \n");
        LOG_DEBUG("%s", buf);
    }

EXIT:
    mPidLock.lock();
    if (mPID >= 0)
        pclose2(fp, mPID);

    mPID = -1;
    mPidLock.unlock();
    LOG_INFO("---- End RTP Player Proc -----\n");    
}


