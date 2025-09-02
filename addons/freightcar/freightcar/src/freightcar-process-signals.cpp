#include    "freightcar.h"
#include    "freightcar-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::signalsOutput(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    // Углы поворота колёсных пар для анимаций
    for(size_t i = 0; i < wheel_rotation_angle.size(); ++i)
        analogSignal[WHEEL_1 + i] =
            static_cast<float>(wheel_rotation_angle[i] / 2.0 / Physics::PI);
}
