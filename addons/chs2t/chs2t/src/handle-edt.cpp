#include    "handle-edt.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HandleEDT::HandleEDT(QObject *parent) : Device(parent)
  , brakeKey(0)
  , releaseKey(0)
  , pos(0)
{
    std::fill(K.begin(), K.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HandleEDT::~HandleEDT()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    double u1 = static_cast<double>(pos == POS_BRAKE);

    double u2 = static_cast<double>(pos == POS_RELEASE);

    Q_bref = K[1] * (pFL - p_bref) * u1 - K[2] * p_bref * u2;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::ode_system(const state_vector_t &Y,
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
void HandleEDT::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    for (size_t i = 1; i < K.size(); ++i)
    {
        QString coeff = QString("K%1").arg(i);
        cfg.getDouble(secName, coeff, K[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HandleEDT::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (getKeyState(brakeKey))
    {
        pos = POS_BRAKE;
    }
    else
    {
        if (getKeyState(releaseKey))
        {
            pos = POS_RELEASE;
        }
        else
        {
            pos = POS_HOLD;
        }
    }
}
