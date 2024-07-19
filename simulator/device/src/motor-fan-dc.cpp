#include    "motor-fan-dc.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::DCMotorFan(QObject *parent) : Device(parent)
    , U_power(0.0)
    , U_nom(1500.0)
    , I(0.0)
    , omega0(188.5)
    , R(9.2)
    , cPhi(6.91)
    , J(2.0)
    , kf(0.0042)
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
DCMotorFan::~DCMotorFan()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::setPowerVoltage(double value)
{
    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotorFan::getPowerCurrent() const
{
    return I;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DCMotorFan::isPowered() const
{
    return is_powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool DCMotorFan::isReady() const
{
    return is_ready;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t DCMotorFan::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float DCMotorFan::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state.createSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::RegulateSoundByOnOff(bool value)
{
    reg_sound_by_on_off = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::RegulateSoundByPitch(bool value)
{
    reg_sound_by_pitch = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    is_powered = (U_power > Physics::ZERO);
    is_ready = (Y[0] > 0.9 * omega0);

    if (reg_sound_by_on_off)
    {
        sound_state.state = is_powered;
    }
    else
    {
        sound_state.state = ( (2.0 * Y[0]) >= omega0 ); // (Y[0] / omega0) >= 0.5
    }

    if (reg_sound_by_pitch)
        sound_state.pitch = static_cast<float>(Y[0] / omega0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    double c_phi = cPhi * sqrt(U_power / U_nom);

    I = (U_power - c_phi * Y[0]) / R;

    double Ma = c_phi * I;

    // Рассчитываем аэродинамический момент сопротивления
    double Mr = kf * Y[0] * Y[0] * Physics::sign(Y[0]);

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "omega0", omega0);
    cfg.getDouble(secName, "R", R);
    cfg.getDouble(secName, "cPhi", cPhi);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "kf", kf);

    cfg.getBool(secName, "RegulateSoundByOnOff", reg_sound_by_on_off);
    cfg.getBool(secName, "RegulateSoundByPitch", reg_sound_by_pitch);
}
