#include    "pneumo-switching-valve.h"

#include    "math-funcs.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitchingValve::SwitchingValve(double working_volume_1,
                               double working_volume_2,
                               QObject *parent)
    : BrakeDevice(parent)
    , V1(working_volume_1)
    , V2(working_volume_2)
    , pOUT(0.0)
    , QIN1(0.0)
    , QIN2(0.0)
    , QOUT(0.0)
    , K1(0.05)
    , A1(1.0)
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
void SwitchingValve::preStep(state_vector_t &Y, double t)
{
    (void) t;

    if (Y[0] > Physics::ZERO)
    {
        Y[0] = std::min(Y[0], 1.0);

        if (ignore_volume1)
        {
            const double u = Y[0];
            const double r = 1.0 - u;
            Y[1] = r * Y[1] + u * pOUT;
        }
        return;
    }
    if (Y[0] < -Physics::ZERO)
    {
        Y[0] = std::max(Y[0], -1.0);

        if (ignore_volume2)
        {
            const double u = -Y[0];
            const double r = 1.0 - u;
            Y[2] = r * Y[2] + u * pOUT;
        }
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::ode_system(const state_vector_t &Y,
                                state_vector_t &dYdt,
                                double t)
{
    (void) t;

    // Перемещение клапана
    dYdt[0] = (Y[1] - Y[2]) * A1;

    // Клапан открыт со стороны первого входа
    if (Y[0] > Physics::ZERO)
    {
        // Переток через первую камеру
        const double u = Y[0];
        const double r = ignore_volume1 ? (1.0 - u) : 1.0;
        const double Q1 = r * u * (pOUT - Y[1]) * K1;

        dYdt[1] = (r * QIN1 + Q1) / V1;
        QOUT = (ignore_volume1 ? (u * QIN1) : 0.0) - Q1;

        // Поток во вторую камеру
        dYdt[2] = QIN2 / V2;
        return;
    }

    // Клапан открыт со стороны второго входа
    if (Y[0] < -Physics::ZERO)
    {
        // Поток в первую камеру
        dYdt[1] = QIN1 / V2;

        // Переток через вторую камеру
        const double u = -Y[0];
        const double r = ignore_volume2 ? (1.0 - u) : 1.0;
        const double Q2 = r * u * (pOUT - Y[2]) * K1;

        dYdt[2] = (r * QIN2 + Q2) / V1;
        QOUT = (ignore_volume2 ? (u * QIN2) : 0.0) - Q2;
        return;
    }

    dYdt[1] = QIN1 / V1;
    dYdt[2] = QIN2 / V2;
    QOUT = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitchingValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double tmp1 = 0.0;
    cfg.getDouble(secName, "V1", tmp1);
    if (tmp1 > 1e-3)
        V1 = tmp1;

    double tmp2 = 0.0;
    cfg.getDouble(secName, "V2", tmp2);
    if (tmp2 > 1e-3)
        V2 = tmp2;

    cfg.getDouble(secName, "K1", K1);
    cfg.getDouble(secName, "A1", A1);

    cfg.getBool(secName, "IgnoreVolume1", ignore_volume1);
    cfg.getBool(secName, "IgnoreVolume2", ignore_volume2);
}
