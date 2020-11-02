#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepCabineControls(double t, double dt)
{
    km->setControl(keys);
    km->step(t, dt);

    tumbler_field_weak1.setControl(keys);
    tumbler_field_weak1.step(t, dt);

    tumbler_field_weak2.setControl(keys);
    tumbler_field_weak2.step(t, dt);

    tumbler_water_zaluzi.setControl(keys);
    tumbler_water_zaluzi.step(t, dt);

    tumbler_oil_zaluzi.setControl(keys);
    tumbler_oil_zaluzi.step(t, dt);
}
