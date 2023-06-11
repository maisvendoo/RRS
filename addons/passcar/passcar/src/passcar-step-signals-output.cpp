#include    "passcar.h"
#include    "passcar-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::stepSignalsOutput()
{
    analogSignal[WHEEL_1] = static_cast<float>(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(wheel_rotation_angle[3] / 2.0 / Physics::PI);

    float generator_axis = static_cast<float>(wheel_rotation_angle[2] * ip / 2.0 / Physics::PI);
    analogSignal[GEN_MUFTA1] = generator_axis;
    analogSignal[GEN_KARDAN] = generator_axis;
    analogSignal[GEN_AXIS] = generator_axis;
}
