#include    "coupling-operating-rod.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OperatingRod::OperatingRod(QObject *parent) : Device(parent)
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
void OperatingRod::setKeySymbol(std::uint16_t key_symbol)
{
    keyCode = key_symbol;
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
    (void) t;
    Y[0] = std::clamp(Y[0], -1.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    (void) t;
    dYdt[0] = sign(ref_operating_state - Y[0]) / motion_time;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::stepKeysControl(double t, double dt)
{
    (void) t;
    (void) dt;

    // Проверяем управляющий сигнал от заданной клавиши
    if (getKeyState(keyCode))
    {
        // Проверяем фиксацию расцепляющего положения
        if (is_fixed_uncoupling)
        {
            // Сбрасывает фиксацию повторное нажатие клавиши без Ctrl и Shift
            if (!(was_keyCode || isShift() || isControl()))
            {
                // Нормальное положение
                ref_operating_state = 1.0;
                is_fixed_uncoupling = false;
            }
            else
            {
                ref_operating_state = -1.0;
            }
        }
        else
        {
            // Shift - команда на расцепление
            if (isShift())
            {
                // Если ещё и Ctrl - фиксируем рычаг в расцепляющем положении
                if (isControl())
                {
                    ref_operating_state = -1.0;
                    if ((getY(0) + 1.0) <= Physics::ZERO)
                        is_fixed_uncoupling = true;
                }
                else
                {
                    // Проверяем натяжение сцепок
                    if (coupling_force <= max_operating_force)
                    {
                        // Расцепляющее положение
                        ref_operating_state = -1.0;
                    }
                    else
                    {
                        // Положение натянутой цепочки, но расцепление невозможно
                        ref_operating_state = min(0.0, getY(0));
                    }
                }
            }
            else
            {
                // Нормальное положение
                ref_operating_state = 1.0;
            }
        }
        was_keyCode = true;
    }
    else
    {
        if (!is_fixed_uncoupling)
        {
            // Нормальное положение
            ref_operating_state = 1.0;
        }
        was_keyCode = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OperatingRod::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "MaxOperatingForce", max_operating_force);

    double tmp = 0.0;
    cfg.getDouble(secName, "MotionTime", tmp);
    if (tmp > 0.0)
        motion_time = tmp;
}
