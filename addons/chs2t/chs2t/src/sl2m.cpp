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
  , Dk(1.25)
  , sound_speed(2.0)
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float SL2M::getArrowPos() const
{
    return arrow_pos;
}

float SL2M::getShaftPos() const
{
    return shaft_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::preStep(state_vector_t &Y, double t)
{
    omega_s = ip * omega;

    double velocity = omega * Dk / 2.0 + cut(1000.0 * omega, -wear_gap, wear_gap) * sin(Y[1]);

    arrow_pos = cut(static_cast<float>(abs(velocity) * Physics::kmh / max_speed), 0.0f, 1.0f);

    double shaft_angle = Y[0];

    shaft_pos = static_cast<float>(abs(shaft_angle) / 2.0 / Physics::PI);

    emit soundSetVolume("Skorostemer", static_cast<int>(100 * hs_p(abs(omega) * Dk / 2.0 - sound_speed)));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SL2M::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
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

    cfg.getDouble(secName, "SoundSpeed", sound_speed);

    sound_speed = sound_speed / Physics::kmh;

    cfg.getDouble(secName, "FreqCoeff", freq_coeff);
}


