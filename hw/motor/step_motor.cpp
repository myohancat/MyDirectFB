#include "step_motor.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ZOOM_SLEEP_FOR_CLOCK 800 /* Micro Second */

static int gGpioStepCCW[8][4] =
{
    { 1, 0, 0, 0 },
    { 1, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 }
};

static int gGpioStepCW[8][4] =
{
    { 1, 0, 0, 1 },
    { 0, 0, 0, 1 },
    { 0, 0, 1, 1 },
    { 0, 0, 1, 0 },
    { 0, 1, 1, 0 },
    { 0, 1, 0, 0 },
    { 1, 1, 0, 0 },
    { 1, 0, 0, 0 }
};

StepMotor* StepMotor::open(int gpio1, int gpio2, int gpio3, int gpio4)
{
    GPIO* gpios[4] = { nullptr, };
    gpios[0] = GPIO::open(gpio1);
    gpios[1] = GPIO::open(gpio2);
    gpios[2] = GPIO::open(gpio3);
    gpios[3] = GPIO::open(gpio4);

    if (!gpios[0] || !gpios[1] || !gpios[2] || !gpios[3])
    {
        for (int ii  =0; ii < 4; ii++)
            if (gpios[ii]) delete gpios[ii];

        return NULL;
    }

    return new StepMotor(gpios);
}

StepMotor::StepMotor(GPIO** gpios)
{
    char path[1024];
    for (int ii = 0; ii < 4; ii++)
    {
        mGpios[ii] = gpios[ii];
        mGpios[ii]->setOutDir(GPIO_DIR_OUT);

        sprintf(path, "%s/value", gpios[ii]->getPath());        
        mFds[ii] = ::open(path, O_WRONLY);
    }
    reset();
}

StepMotor::~StepMotor()
{
    for (int ii = 0; ii < 4; ii++)
    {
        if (mGpios[ii])
            delete(mGpios[ii]);
        if (mFds[ii] != -1)
            ::close(mFds[ii]);
    }    
}

inline void StepMotor::reset() const
{
    for (int ii = 0; ii < 4; ii++)
    {
        ::write(mFds[ii], "0", 1);
    }
    usleep(ZOOM_SLEEP_FOR_CLOCK);
}

inline void StepMotor::cw() const
{
    char data;
    for (int ii = 0; ii < 8; ii++)
    {
        int *pins = gGpioStepCW[ii];
        for (int jj = 0; jj < 4; jj++)
        {
            data = '0' + pins[jj];
            ::write(mFds[jj], &data, 1);
        }
        usleep(ZOOM_SLEEP_FOR_CLOCK);
    }
}

inline void StepMotor::ccw() const
{
    char data;
    for (int ii = 0; ii < 8; ii++)
    {
        int *pins = gGpioStepCCW[ii];
        for (int jj = 0; jj < 4; jj++)
        {
            data = '0' + pins[jj];
            ::write(mFds[jj], &data, 1);
        }
        usleep(ZOOM_SLEEP_FOR_CLOCK);
    }
}

void StepMotor::cw(int cnt)
{
    for (int ii = 0; ii < cnt; ii++)
        cw();

    reset();
}

void StepMotor::ccw(int cnt)
{
    for (int ii = 0; ii < cnt; ii++)
        ccw();

    reset();
}
