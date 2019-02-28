#ifndef     CONTROLS_H
#define     CONTROLS_H

#include    "trigger.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IncBrakePos : public Trigger<int>
{
private:

    void operate(int &value)
    {
        value++;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DecBrakePos : public Trigger<int>
{
private:

    void operate(int &value)
    {
        value--;
    }
};

#endif // CONTROLS_H
