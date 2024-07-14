#include    "motor-fan-ac.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorFan::ACMotorFan(QObject *parent) : Device(parent)
    , U_power(0.0)
    , U_nom(380.0)//
    , omega0(157.08)//
    , Mmax(611.5)//
    , s_kr(0.08)//
    , J(2.0)//
    , kf(0.0154)//
    , is_powered(false)
    , is_ready(false)
    , sound_state(sound_state_t())
    , reg_sound_by_on_off(false)
    , reg_sound_by_pitch(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorFan::~ACMotorFan()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::setPowerVoltage(double value)
{
    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ACMotorFan::isPowered() const
{
    return is_powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ACMotorFan::isReady() const
{
    return is_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ACMotorFan::getSoundState() const
{
    return sound_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::RegulateSoundByOnOff(bool value)
{
    reg_sound_by_on_off = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::RegulateSoundByPitch(bool value)
{
    reg_sound_by_pitch = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    is_powered = (U_power > 0.9 * U_nom);
    is_ready = (Y[0] > 0.9 * omega0);

    if (reg_sound_by_on_off)
    {
        sound_state.play = is_powered;
    }
    else
    {
        sound_state.play = ( (2.0 * Y[0]) >= omega0 ); // (Y[0] / omega0) >= 0.5
    }

    if (reg_sound_by_pitch)
        sound_state.pitch = static_cast<float>(Y[0] / omega0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    // Расчитываем текущее скольжение ротора
    double s = 1 - Y[0] / omega0;

    // Рачитываем максимальный момент при данном напряжении питания
    double M_maximal = Mmax * pow(U_power / U_nom, 2.0);

    // Расчитываем электромагнитный момент (формула Клосса)
    double Ma = 2 * M_maximal / ( s / s_kr + s_kr / s );

    // Рассчитываем аэродинамический момент сопротивления
    double Mr = kf * Y[0] * Y[0] * Physics::sign(Y[0]);

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorFan::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "U_nom", U_nom);
    cfg.getDouble(secName, "omega0", omega0);
    cfg.getDouble(secName, "Mmax", Mmax);
    cfg.getDouble(secName, "s_kr", s_kr);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "kf", kf);

    cfg.getBool(secName, "RegulateSoundByOnOff", reg_sound_by_on_off);
    cfg.getBool(secName, "RegulateSoundByPitch", reg_sound_by_pitch);
}
