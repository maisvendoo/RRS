#include    "es1-motor.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ES1Motor::ES1Motor() : Vehicle ()
  , traction_level(0.0)
  , inc_loc(false)
  , dec_loc(false)
  , auto_reg(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ES1Motor::~ES1Motor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ES1Motor::keyProcess()
{
    double traction_step = 0.1;

    if (keys[KEY_A] && !inc_loc)
    {
        traction_level +=  traction_step;
        inc_loc = true;
    }
    else
    {
        inc_loc = false;
    }

    if (keys[KEY_D] && !dec_loc)
    {
        traction_level -=  traction_step;
        dec_loc = true;
    }
    else
    {
        dec_loc = false;
    }

    enSpeedReg.process(keys[KEY_E], auto_reg);

    incRefSpeed.process(keys[KEY_Q], vz);
    decRefSpeed.process(keys[KEY_W], vz);

    vz = cut(vz, 0.0, 160.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ES1Motor::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (auto_reg)
    {
        double K1 = 1.6;
        traction_level = K1 * (vz / Physics::kmh - velocity);
    }

    traction_level = Physics::cut(traction_level, -1.0, 1.0);

    double actor_torque = traction_char(velocity) * wheel_diameter / num_axis / 2.0;

    double trac_torque = actor_torque * pf(traction_level);
    double brake_troque = actor_torque * nf(traction_level);

    for (size_t i = 1; i < Q_a.size(); i++)
    {
        Q_a[i] = trac_torque;
        Q_r[i] = brake_troque;
    }

    DebugMsg = QString("Время: %1 Коорд.: %5 Зад. Скор.: %4 Скор.: %2 Тяга: %3 ")
            .arg(t, 10, 'f', 1)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(traction_level, 4, 'f', 1)
            .arg(vz, 6, 'f', 1)
            .arg(railway_coord, 7, 'f', 2);

    if (next_vehicle != nullptr)
    {
        DebugMsg += next_vehicle->getDebugMsg();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ES1Motor::traction_char(double v)
{
    double max_traction = 127.5e3;

    double vn = 81.0 / Physics::kmh;

    if (abs(v) <= vn)
        return max_traction;
    else
        return max_traction * vn / v;
}

GET_VEHICLE(ES1Motor)
