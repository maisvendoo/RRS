#include     "dc-pulse-converter.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCPulseConverter::DCPulseConverter(QObject *parent) : Device(parent)
  , key1(false)
  , key2(false)
  , dI(1.0)
  , I_ref(0.0)
  , I(0.0)
  , U_in(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCPulseConverter::~DCPulseConverter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCPulseConverter::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if (I < I_ref - dI)
    {
        key1 = true;
    }

    if (I > I_ref + dI)
    {
        key1 = false;
    }

    key2 = !key1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCPulseConverter::ode_system(const state_vector_t &,
                                state_vector_t &,
                                double)
{

}

void DCPulseConverter::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

