#include    "sl2m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SL2M::SL2M(QObject *parent) : Device(parent)
    , omega(0.0)
    , ip(3.0 * Physics::PI / 50.0)
    , omega_s(0.0)
    , wear_gap(1.0)
    , max_speed(150.0)
    , arrow_pos(0.0f)
    , Dk(1.25)
    , speed_begin_sound(2.0)
    , omega_begin_sound(speed_begin_sound * 2.0 / Dk / Physics::kmh)
    , shaft_pos(0.0)
    , freq_coeff(1.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SL2M::~SL2M()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::setOmega(double value)
{
    omega = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::setWheelDiameter(double diam)
{
    Dk = diam;
    omega_begin_sound = speed_begin_sound * 2.0 / Dk / Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getArrowPos() const
{
    return arrow_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getShaftPos() const
{
    return shaft_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t SL2M::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t(abs(omega) >= omega_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(abs(omega) >= omega_begin_sound);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    omega_s = ip * omega;

    double velocity = abs(omega) * Dk / 2.0 + min(1000.0 * abs(omega), wear_gap) * sin(Y[1]);

    arrow_pos = static_cast<float>(min(velocity * Physics::kmh / max_speed, 1.0));

    double shaft_angle = Y[0];

    shaft_pos = static_cast<float>(shaft_angle / 2.0 / Physics::PI);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = omega_s;
    dYdt[1] = freq_coeff * omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "MaxSpeed", max_speed);
    cfg.getDouble(secName, "WearGap", wear_gap);

    wear_gap = wear_gap / Physics::kmh;

    cfg.getDouble(secName, "SoundSpeed", speed_begin_sound);

    cfg.getDouble(secName, "FreqCoeff", freq_coeff);
}


