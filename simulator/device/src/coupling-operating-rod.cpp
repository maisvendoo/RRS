#include    "coupling-operating-rod.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OperatingRod::OperatingRod(int key_code, QObject *parent) : Device(parent)
  , keyCode(key_code)
  , ref_operating_state(1.0)
  , is_fixed_uncoupling(false)
  , coupling_force(0.0)
  , max_operating_force(Physics::ZERO)
  , motion_time(0.1)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OperatingRod::~OperatingRod()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::setKeyCode(int key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::setCouplingForce(double force)
{
    coupling_force = force;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
double OperatingRod::getOperatingState() const
{
    return getY(0);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
bool OperatingRod::isFixedUncoupled() const
{
    return is_fixed_uncoupling;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void OperatingRod::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Y[0] = cut(Y[0], -1.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    Q_UNUSED(t)
    dYdt[0] = sign(ref_operating_state - Y[0]) / motion_time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    //TODO// фиксация расцепного рычага в расцепляющем положении

    // Проверяем управляющий сигнал
    if (getKeyState(keyCode) && isShift())
        // Проверяем натяжение сцепок
        if (coupling_force <= max_operating_force)
            // Расцепляющее положение
            ref_operating_state = -1.0;
        else
            // Положение натянутой цепочки, но расцепление невозможно
            ref_operating_state = min(0.0, getY(0));
    else
        // Нормальное положение
        ref_operating_state = 1.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    QString secName = "Device";

    cfg.getDouble(secName, "MaxOperatingForce", max_operating_force);

    double tmp = 0.0;
    cfg.getDouble(secName, "MotionTime", tmp);
    if (tmp > 0.0)
        motion_time = tmp;
}
