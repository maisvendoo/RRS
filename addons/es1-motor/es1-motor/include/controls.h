#ifndef     CONTROLS_H
#define     CONTROLS_H

#include    "trigger.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EnableSpeedReg : public Trigger<bool>
{
private:

    void operate(bool &value)
    {
        value = !value;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IncRefSpeed : public Trigger<double>
{
private:

    void operate(double &value)
    {
        value += 1.0;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DecRefSpeed : public Trigger<double>
{
private:

    void operate(double &value)
    {
        value -= 1.0;
    }
};

#endif // CONTROLS_H
