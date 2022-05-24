#include "irq.h"

#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"

#define SYS_FS_DIR "/sys/class/gpio"

void IRQ::setListener(IIRQListener* listener)
{
    mListener = listener;
}

IRQ::IRQ(GPIO* gpio, GPIO_Edge_e egde, bool activeLow)
    : mEdge(egde), 
      mActiveLow(activeLow)
{
    pipe(mPipe);
    mGPIO = gpio;
    mGPIO->setEdge(GPIO_EDGE_NONE);
    mGPIO->setActiveLow(activeLow);

    start();
}

IRQ::~IRQ()
{
    stop();

    if(mPipe[0] >= 0)
        close(mPipe[0]);
    if(mPipe[1] >= 0)
        close(mPipe[0]);

    if (!mGPIO)
        delete mGPIO;
}

void IRQ::disable()
{
#if 0
    mGPIO->setEdge(GPIO_EDGE_NONE);
#else
    write(mPipe[1], "D", 1);
#endif
}

void IRQ::enable()
{
#if 0
    mGPIO->setEdge(mEdge);
#else
    write(mPipe[1], "E", 1);
#endif
}

#define POLL_TIMEOUT   (1000*1000)
void IRQ::run()
{
    int ret = 0;
    struct pollfd fds[2];
    nfds_t nfds = 2;

    fds[0].fd = mPipe[0];
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    fds[1].fd = mFD;
    fds[1].events = POLLPRI | POLLERR;
    fds[1].revents = 0;

    char dummy;
    ::read(mFD, &dummy, 1);

__TRACE_FUNC__;
    while(!mExitTask)
    {
        ret = ::poll(fds, nfds, POLL_TIMEOUT);
        if (ret == 0)
        {
            //LOG_WARN("poll timeout - %d msec. GIVE UP.\n", POLL_TIMEOUT);
            continue;
        }

        if (ret < 0)
        {
            LOG_ERROR("poll failed. ret=%d, errno=%d.\n", ret, errno);
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            char data;
            ::read(mPipe[0], &data, 1);

            switch(data)
            {
                case 'D':
                    mGPIO->setEdge(GPIO_EDGE_NONE);
                    LOG_TRACE("IRQ %d disabled.\n", mGPIO->getNumber());
                    break;
                case 'E':
                    mGPIO->setEdge(mEdge);
                    LOG_TRACE("IRQ %d enabled.\n", mGPIO->getNumber());
                    break;
                case 'T':
                    LOG_INFO("Termiate IRQ Proc\n");
                    continue;
            }
        }

        if (fds[1].revents & POLLPRI)
        {

            LOG_TRACE("[[[[ INTERRUPTED ]]]] IRQ %d\n", mGPIO->getNumber());
            if (mListener)
                mListener->onInterrupted(this);

            char dummy;
            ::read(mFD, &dummy, 1);
        }

        if ( fds[0].revents & POLLRDHUP )
            LOG_ERROR("POLLRDHUP.\n");
        if ( fds[0].revents & POLLERR )
            LOG_ERROR("POLLERR.\n");
        if ( fds[0].revents & POLLHUP )
            LOG_ERROR("POLLHUP.\n");
        if ( fds[0].revents & POLLNVAL )
            LOG_ERROR("POLLNVAL.\n");
    }
}

bool IRQ::onPreStart()
{
    mExitTask = false;
    char path[1024];
    sprintf(path, "%s/value", mGPIO->getPath());
    mFD = ::open(path, O_RDONLY);
    if (mFD == -1)
        return false;

    return true;
}

void IRQ::onPreStop()
{
    mExitTask = true;
    write(mPipe[1], "T", 1);
}

void IRQ::onPostStop()
{
    mExitTask = true;
    if (mFD != -1)
    {
        ::close(mFD);
        mFD = -1;
    }
}
