#include    "motor-compressor-dc.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::DCMotorCompressor(QObject *parent) : Device(parent)
  , pFL(0.0)
  , QFL(0.0)
  , U_power(0.0)
  , I(0.0)
  , omega0(157.08)
  , R(56.9)
  , cPhi(14.2)
  , J(2.0)
  , Mxx(50.0)
  , K_pressure(0.0061)
  , K_flow(0.005)
  , is_powered(false)
  , soundName("Motor_Compressor")
  , reg_sound_by_on_off(false)
  , reg_sound_by_pitch(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::~DCMotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotorCompressor::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::setPowerVoltage(double value)
{
    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotorCompressor::getPowerCurrent() const
{
    return I;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DCMotorCompressor::isPowered() const
{
    return is_powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::setSoundName(const QString &value)
{
    soundName = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::RegulateSoundByOnOff(bool value)
{
    reg_sound_by_on_off = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::RegulateSoundByPitch(bool value)
{
    reg_sound_by_pitch = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    QFL = K_flow * pf(K_pressure * Y[0] - pFL);

    bool powered_prev = is_powered;
    is_powered = (U_power > Physics::ZERO);

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
void DCMotorCompressor::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    I = (U_power - cPhi * Y[0]) / R;

    double Ma = cPhi * I;

    double Mr = Physics::fricForce(Mxx, 0.1 * Y[0]);

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "omega0", omega0);
    cfg.getDouble(secName, "R", R);
    cfg.getDouble(secName, "cPhi", cPhi);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "Mc", Mxx);

    cfg.getDouble(secName, "K_pressure", K_pressure);
    cfg.getDouble(secName, "K_flow", K_flow);

    cfg.getString(secName, "SoundName", soundName);
    cfg.getBool(secName, "RegulateSoundByOnOff", reg_sound_by_on_off);
    cfg.getBool(secName, "RegulateSoundByPitch", reg_sound_by_pitch);
}
