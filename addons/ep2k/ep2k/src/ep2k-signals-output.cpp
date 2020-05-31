#include    "ep2k.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::stepSignals(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    analogSignal[PANT1] = pantogrph[PANT_FWD]->getHeight();
    analogSignal[PANT2] = pantogrph[PANT_BWD]->getHeight();

    analogSignal[WHEEL_1] = static_cast<float>(dir * wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(dir * wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(dir * wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(dir * wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(dir * wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(dir * wheel_rotation_angle[5] / 2.0 / Physics::PI);
}
