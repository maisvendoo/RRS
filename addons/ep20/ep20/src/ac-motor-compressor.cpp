#include    "ac-motor-compressor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorCompressor::ACMotorCompressor(QString config_path, QObject *parent) : Device(parent)
  , p(0.0)
  , Q(0.0)
  , p0(1.5)
  , Mmax(455.8)
  , s_kr(0.154)
  , Un(380.0)
  , U_power(0.0)
  , omega0(157.08)
  , J(0.5)
  , Mxx(50.0)
  , Vnk(0.05)

{
    std::fill(K.begin(), K.end(), 0);

    load_config(config_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ACMotorCompressor::~ACMotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::setExternalPressure(double press)
{
    p = press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ACMotorCompressor::getAirFlow() const
{
    return Q;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::setU_power(double value)
{
    U_power = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Q = K[4] * pf(Y[1] - p);

    emit soundSetPitch("Motor_Compressor", static_cast<float>(Y[0] / omega0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    // Расчитывает текущее скольжение ротора
    double s = 1 - Y[0] / omega0;

    // Рачитываем максимальный момент при данном напряжении питания
    double M_maximal = Mmax * pow(U_power / Un, 2.0);

    // Расчитываем электромагнитный момент (формула Клосса)
    double Ma = 2 * M_maximal / ( s / s_kr + s_kr / s );

    double Mr = Physics::fricForce(Mxx, Y[0]);

    double Qnk =  K[1] * Y[0] - K[2] * Y[1] - K[3] * pf(Y[1] - p);

    dYdt[0] = (Ma - Mr) / J;

    dYdt[1] = Qnk / Vnk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ACMotorCompressor::load_config(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Device";

        int order = 1;
        if (cfg.getInt(secName, "Order", order))
        {
            y.resize(static_cast<size_t>(order));
            std::fill(y.begin(), y.end(), 0);
        }

        for (size_t i = 1; i < K.size(); ++i)
        {
            QString coeff = QString("K%1").arg(i);
            cfg.getDouble(secName, coeff, K[i]);
        }
    }
}
