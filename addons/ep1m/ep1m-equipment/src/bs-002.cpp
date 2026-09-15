#include    "bs-002.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SignalizationModule::SignalizationModule(QObject *parent) : Device(parent)
  , Upow(0.0)
{
    std::fill(lamp_state.begin(), lamp_state.end(), false);
    std::fill(lamp_input.begin(), lamp_input.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SignalizationModule::~SignalizationModule()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SignalizationModule::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (Upow < 49.0)
    {
        std::fill(lamp_state.begin(), lamp_state.end(), false);
        return;
    }

    lamp_state = lamp_input;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SignalizationModule::ode_system(const state_vector_t &Y,
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
void SignalizationModule::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
