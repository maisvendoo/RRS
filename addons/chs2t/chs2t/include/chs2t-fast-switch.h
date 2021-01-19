#ifndef CHS2T_FAST_SWITCH_H
#define CHS2T_FAST_SWITCH_H

#include "protective-device.h"

class CHS2TFastSwitch : public ProtectiveDevice
{
public:

    CHS2TFastSwitch(QObject *parent = Q_NULLPTR)
        : ProtectiveDevice(parent)
        , old_state(true)
    {

    }

    ~CHS2TFastSwitch() {}

private:

    bool old_state;

    void stepDiscrete(double t, double dt)
    {
        Q_UNUSED(t)
        Q_UNUSED(dt)

        if (getState() != old_state)
        {
            if (!getState())
                emit soundPlay("BV-on");
            else
                emit soundPlay("BV-off");
        }

        old_state = getState();
    }
};


#endif // CHS2T_FAST_SWITCH_H
