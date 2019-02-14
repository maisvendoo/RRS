#include    "sapsan-motor.h"
#include    "physics.h"

SapsanMotor::SapsanMotor() : Vehicle ()
  , traction_level(0.0)
  , inc_loc(false)
  , dec_loc(false)
{

}

SapsanMotor::~SapsanMotor()
{

}

void SapsanMotor::keyProcess()
{
    double traction_step = 0.1;

    if (keys[97] && !inc_loc)
    {
        traction_level +=  traction_step;
        inc_loc = true;
    }
    else
    {
        inc_loc = false;
    }

    if (keys[100] && !dec_loc)
    {
        traction_level -=  traction_step;
        dec_loc = true;
    }
    else
    {
        dec_loc = false;
    }
}

void SapsanMotor::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    traction_level = Physics::cut(traction_level, -1.0, 1.0);

    for (size_t i = 0; i < Q_a.size(); i++)
    {
        double torque = traction_level * traction_char(velocity) * wheel_diameter / num_axis / 2.0;
        Q_a[i] = torque;
    }
}

double SapsanMotor::traction_char(double v)
{
    double max_traction = 355e3 / 4.0;

    double vn = 81.0 / Physics::kmh;

    if (abs(v) <= vn)
        return max_traction;
    else
        return max_traction * vn / v;
}

GET_VEHICLE(SapsanMotor)
