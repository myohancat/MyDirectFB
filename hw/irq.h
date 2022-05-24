#ifndef __IRQ_H_
#define __IRQ_H_

#include "task.h"
#include "gpio.h"

class IRQ;

class IIRQListener
{
public:
    virtual ~IIRQListener() { };

    virtual void onInterrupted(const IRQ* irq) = 0;
};

class IRQ : public Task
{
public:
    IRQ(GPIO* gpio, GPIO_Edge_e egde, bool activeLow);
    virtual ~IRQ();

    void disable();
    void enable();

    void setListener(IIRQListener* listener);

private:

    void run();

    bool onPreStart();
    void onPreStop();
    void onPostStop();
private:
    GPIO* mGPIO = nullptr;
    int   mFD  = -1;

    bool mExitTask = false;

    int mPipe[2] = { -1, -1 };

    IIRQListener* mListener = nullptr;

    GPIO_Edge_e   mEdge;
    bool          mActiveLow;
};


#endif //__GPIO_H_
