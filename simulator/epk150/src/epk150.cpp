#include    "epk150.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStopEPK150::AutoTrainStopEPK150(QObject *parent)
    : AutoTrainStop(parent)
{
    std::fill(K.begin(), K.end(), 0.0);
    std::fill(k.begin(), k.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStopEPK150::ode_system(const state_vector_t &Y,
                                     state_vector_t &dYdt,
                                     double t)
{
    Q_UNUSED(t)

    dYdt[0] = 0;

    dYdt[1] = 0;

    dYdt[2] = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStopEPK150::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }

    for (size_t i = 1; i < k.size(); ++i)
    {
        QString coeff = QString("k%1").arg(i);
        cfg.getDouble(secName, coeff, k[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_AUTO_TRAIN_STOP(AutoTrainStopEPK150)
