#include    "motor-compressor-ac.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorCompressor::ACMotorCompressor(QObject *parent) : Device(parent)
  , pFL(0.0)
  , QFL(0.0)
  , U_power(0.0)
  , U_nom(380.0)
  , omega0(157.08)
  , Mmax(455.8)
  , s_kr(0.154)
  , J(2.0)
  , Mxx(50.0)
  , K_pressure(0.0077)
  , K_flow(0.02)
  , is_powered(false)
  , soundName("Motor_Compressor")
  , reg_sound_by_on_off(false)
  , reg_sound_by_pitch(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorCompressor::~ACMotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ACMotorCompressor::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::setPowerVoltage(double value)
{
    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ACMotorCompressor::isPowered() const
{
    return is_powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::setSoundName(const QString &value)
{
    soundName = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::RegulateSoundByOnOff(bool value)
{
    reg_sound_by_on_off = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::RegulateSoundByPitch(bool value)
{
    reg_sound_by_pitch = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    QFL = K_flow * pf(K_pressure * Y[0] - pFL);

    bool powered_prev = is_powered;
    is_powered = (U_power > 0.9 * U_nom);

    if (reg_sound_by_on_off)
    {
        if (is_powered && !powered_prev)
            emit soundPlay(soundName);

        if (!is_powered && powered_prev)
            emit soundStop(soundName);
    }

    if (reg_sound_by_pitch)
        emit soundSetPitch(soundName, static_cast<float>(Y[0] / omega0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::ode_system(const state_vector_t &Y,
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

    double Mr = Physics::fricForce(Mxx, 0.1 * Y[0]);

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "U_nom", U_nom);
    cfg.getDouble(secName, "omega0", omega0);
    cfg.getDouble(secName, "Mmax", Mmax);
    cfg.getDouble(secName, "s_kr", s_kr);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "Mxx", Mxx);

    cfg.getDouble(secName, "K_pressure", K_pressure);
    cfg.getDouble(secName, "K_flow", K_flow);

    cfg.getString(secName, "SoundName", soundName);
    cfg.getBool(secName, "RegulateSoundByOnOff", reg_sound_by_on_off);
    cfg.getBool(secName, "RegulateSoundByPitch", reg_sound_by_pitch);
}
