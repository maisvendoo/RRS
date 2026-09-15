#include    "reversor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Reversor::Reversor(QObject *parent) : Device(parent)
  , state(0)
  , shaft_omega_max(2.0)
  , omega(0.0)
  , forward_valve(false)
  , backward_valve(false)
  , pos_ref(0)
  , K(10.0)
  , eps(0.99)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Reversor::~Reversor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t Reversor::getSoundState(size_t idx) const
{
    (void) idx;
    return reversor_sound;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Reversor::getSoundSignal(size_t idx) const
{
    (void) idx;
    return reversor_sound.createSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Reversor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    int pos_ref_old = pos_ref;
    pos_ref = static_cast<int>(forward_valve) - static_cast<int>(backward_valve);

    if (pos_ref != pos_ref_old)
    {
        reversor_sound.play();
    }

    omega = shaft_omega_max * cut(K * (pos_ref - Y[0]), -1.0, 1.0);

    state = static_cast<int>( hs_p(Y[0] - eps) - hs_n(Y[0] + eps) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Reversor::ode_system(const state_vector_t &Y,
                          state_vector_t &dYdt,
                          double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Reversor::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "ShaftOmega", shaft_omega_max);
    cfg.getDouble(secName, "K", K);
    cfg.getDouble(secName, "eps", eps);
}
