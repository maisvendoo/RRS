#include    "km2202.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::ControllerKM2202(QObject *parent) : TractionController(parent)
  , position(0)
  , reversor_dir(0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ControllerKM2202::~ControllerKM2202()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::ode_system(const state_vector_t &Y,
                                  state_vector_t &dYdt,
                                  double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    incTimer = new Timer(0.3);
    decTimer = new Timer(0.3);

    connect(incTimer, &Timer::process, this, &ControllerKM2202::inc_trac_position);
    connect(decTimer, &Timer::process, this, &ControllerKM2202::dec_trac_position);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_A))
    {
        incTimer->start();
    }
    else
    {
        incTimer->stop();
    }

    if (getKeyState(KEY_D))
    {
        if (getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R))
            position = 0;
         else
            decTimer->start();
    }
    else
    {
        decTimer->stop();
    }

    incTimer->step(t, dt);
    decTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::inc_trac_position()
{
    position++;
    position = cut(position, static_cast<int>(MIN_POSITION), static_cast<int>(MAX_POSITION));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::dec_trac_position()
{
    position--;
    position = cut(position, static_cast<int>(MIN_POSITION), static_cast<int>(MAX_POSITION));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::inc_reversor_dir()
{
    reversor_dir++;
    reversor_dir = cut(reversor_dir, -1, 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ControllerKM2202::dec_reversor_dir()
{
    reversor_dir--;
    reversor_dir = cut(reversor_dir, -1, 1);
}


GET_TRACTION_CONTROLLER(ControllerKM2202)
