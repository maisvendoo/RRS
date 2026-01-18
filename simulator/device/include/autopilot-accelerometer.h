#ifndef     AUTOPILOT_ACCELEROMETER_H
#define     AUTOPILOT_ACCELEROMETER_H

#include    <array>
#include    <cstddef>
#include    <physics.h>
#include    <device-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Accelerometer
{
public:

    Accelerometer() = default;

    ~Accelerometer() = default;

    void step(double t, double dt);

    void setVelocity(double v_kmh)
    {
        v = v_kmh / Physics::kmh;
    }

    double value()
    {
        return acceleration;
    }

private:

    enum
    {
        DIFF_NUM = 3
    };

    /// Мвссив значений скоростей для численного дифференцирования
    std::array<double, DIFF_NUM> v_i = {0.0, 0.0, 0.0};

    size_t v_count = 0;

    double t_diff = 0.0;

    double acceleration = 0.0;

    double delta_t = 1.0;

    double v = 0.0;
};

#endif
