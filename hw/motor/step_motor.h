#ifndef __STEP_MOTOR_H_
#define __STEP_MOTOR_H_

#include "gpio.h"

class StepMotor 
{
public:
    static StepMotor* open(int gpio1, int gpio2, int gpio3, int gpio4);
    ~StepMotor();

    void reset() const;
    void cw() const;
    void ccw() const;

    void cw(int cnt);
    void ccw(int cnt);

private:
    StepMotor(GPIO** gpios);

private:
    GPIO* mGpios[4] = { nullptr, };
    int   mFds[4]   = { -1, };
};

#endif /* __STEP_MOTOR_H_ */
