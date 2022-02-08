#include    "voltage-regulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VoltageRegulator::VoltageRegulator(QObject *parent) : Device(parent)
  , U_ref(0.0)
  , dU(0.0)
  , U(0.0)
  , Uf(0.0)
  , U_bat(0.0)
{
    K[0] = 0.01;
    K[1] = 0.01;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VoltageRegulator::~VoltageRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VoltageRegulator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Y[0] = cut(Y[0], 0.0, 1.0);

    dU = U_ref - U;

    double s = K[0] * dU + K[1] * Y[0];

    double u = cut(s, 0.0, 1.0);

    Uf = u * U_bat;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VoltageRegulator::ode_system(const state_vector_t &Y,
                                  state_vector_t &dYdt,
                                  double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = dU;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VoltageRegulator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 0; i < K.size(); ++i)
    {
        QString param = QString("K%1").arg(i);
        cfg.getDouble(secName, param, K[i]);
    }
}
