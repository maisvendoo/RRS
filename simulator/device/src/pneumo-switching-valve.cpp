#include    "pneumo-switching-valve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitchingValve::SwitchingValve(double working_volume, QObject *parent)
    : BrakeDevice(parent)
    , V0(working_volume)
    , pOUT(0.0)
    , QIN1(0.0)
    , QIN2(0.0)
    , QOUT(0.0)
    , K1(0.01)
    , K2(0.01)
    , A1(1.0)
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
void SwitchingValve::setInputFlow1(double value)
{
    QIN1 = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SwitchingValve::getPressure1() const
{
    return getY(1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::setInputFlow2(double value)
{
    QIN2 = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SwitchingValve::getPressure2() const
{
    return getY(2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::setOutputPressure(double value)
{
    pOUT = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SwitchingValve::getOutputFlow() const
{
    return QOUT;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::ode_system(const state_vector_t &Y,
                                state_vector_t &dYdt,
                                double t)
{
    Q_UNUSED(t)

    /*double s1 = A1 * (Y[0] - Y[1]);

    double u1 = cut(pf(k * s1), 0.0, 1.0);

    double u2 = cut(nf(k * s1), 0.0, 1.0);

    double Q_out1 = K1 * (Y[0] - p_out) * u1;

    double Q_1sum = Q1 - K2 * Q_out1;

    double Q_out2 = K1 * (Y[1] - p_out) * u2;

    double Q_2sum = Q2 - K2 * Q_out2;

    Q_out = Q_out1 + Q_out2;

    dYdt[0] = Q_1sum / V_work;

    dYdt[1] = Q_2sum / V_work;*/

    // Перемещение клапана
    double v = A1 * (Y[1] - Y[2]);

    double u1 = pf(Y[0]);

    double u2 = nf(Y[0]);

    // Исходящий поток из первой камеры
    double Q1 = K1 * (Y[1] - pOUT) * u1;

    // Исходящий поток из второй камеры
    double Q2 = K1 * (Y[2] - pOUT) * u2;

    QOUT = Q1 + Q2;

    dYdt[0] = v;

    dYdt[1] = (QIN1 - Q1) / V0;

    dYdt[2] = (QIN2 - Q2) / V0;

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    Y[0] = cut(Y[0], -1.0, 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "K1", K1);
    cfg.getDouble(secName, "K2", K2);
    cfg.getDouble(secName, "A1", A1);
    cfg.getDouble(secName, "k1", k);
}
