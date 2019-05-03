#include    "switching-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitchingValve::SwitchingValve(double working_volume, QObject *parent)
    : BrakeDevice(parent)
    , V_work(working_volume)
    , K1(0.01)
    , K2(0.01)
    , A1(1.0)
    , p_out(0.0)
    , Q1(0.0)
    , Q2(0.0)
    , Q_out(0.0)
    , k(1.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitchingValve::~SwitchingValve()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::setInputFlow1(double Q1)
{
    this->Q1 = Q1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::setInputFlow2(double Q2)
{
    this->Q2 = Q2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::setOutputPressure(double p_out)
{
    this->p_out = p_out;
}

double SwitchingValve::getPressure1() const
{
    return getY(0);
}

double SwitchingValve::getPressure2() const
{
    return getY(1);
}

double SwitchingValve::getOutputFlow() const
{
    return Q_out;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::ode_system(const state_vector_t &Y,
                                state_vector_t &dYdt,
                                double t)
{
    Q_UNUSED(t)

    double s1 = A1 * (Y[0] - Y[1]);

    double u1 = cut(pf(k * s1), 0.0, 1.0);

    double u2 = cut(nf(k * s1), 0.0, 1.0);

    double Q_out1 = K1 * (Y[0] - p_out) * u1;

    double Q_1sum = Q1 - K2 * Q_out1;

    double Q_out2 = K1 * (Y[0] - p_out) * u2;

    double Q_2sum = Q2 - K2 * Q_out2;

    Q_out = Q_out1 + Q_out2;

    dYdt[0] = Q_1sum / V_work;

    dYdt[1] = Q_2sum / V_work;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "K1", K1);
    cfg.getDouble(secName, "K2", K2);
    cfg.getDouble(secName, "WorkVolume", V_work);
    cfg.getDouble(secName, "A1", A1);
    cfg.getDouble(secName, "k1", k);
}
