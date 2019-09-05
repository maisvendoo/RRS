 #include    "pressure-regulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PressureRegulator::PressureRegulator(double p_min,
                                     double p_max,
                                     QObject *parent) : Device(parent)
  , p_cur(0.0)
  , p_prev(0.0)
  , p_min(p_min)
  , p_max(p_max)
  , state(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PressureRegulator::~PressureRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::setPressure(double press)
{
    p_prev = p_cur;
    p_cur = press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PressureRegulator::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    double dp = p_cur - p_prev;

    if (p_cur < p_min)
        state = 1.0;

    if (p_cur > p_max)
        state = 0.0;

    if ( (p_cur >= p_min) && (p_cur <= p_max) )
    {
        state = hs_p(dp);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PressureRegulator::ode_system(const state_vector_t &Y,
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
void PressureRegulator::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
