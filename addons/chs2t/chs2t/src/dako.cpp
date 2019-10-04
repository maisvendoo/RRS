#include "dako.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Dako::Dako(QObject* parent) : Device(parent)
  , V1(0.0)
  , Vy(0.0)
  , U1(0.0)
  , U2(0.0)

  , pgr(0.0)
  , Q1(0.0)
  , Qvr(0.0)
  , ptc(0.0)
  , U(0.0)

  , p1(0.0)
  , py(0.0)
  , Qtc(0.0)
  , p_kvt(0.0)

  , A1(0.0)
  , A2(0.0)
  , A3(0.0)

  , K1(0.0)
  , K2(0.0)
  , K3(0.0)
  , K4(0.0)
  , K5(0.0)

  , k_1(0.0)
  , k_2(0.0)
  , k_3(0.0)
  , k_4(0.0)

  , EDT_state(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Dako::~Dako()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Dako::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)

    double s1 = (pf((U - U2) * k_1));
    double s2 = cut(s1, 0.0, 1.0);

    double s3 = (nf((U - U1) * k_2));
    double s4 = cut(s3, 0.0, 1.0);

    double s5 = Y[0] * A1 + Y[1] * A2 - ptc * A3;

    double s6 = (pf(s5) * k_3);
    double s7 = cut(s6, 0.0, 1.0);

    double s8 = (nf(s5) * k_4);
    double s9 = cut(s8, 0.0, 1.0);

    Qtc = (pgr - ptc) * K4 * s7 - ptc * K3 * s9;

    dYdt[0] = Qvr * K5 / Vy;

    dYdt[1] = ((p_kvt - Y[1]) * K1 * s2 - Y[1] * K2 * s4) / V1;
}

void Dako::preStep(state_vector_t& Y, double t)
{
    EDT_state = (U * Physics::kmh > 50);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Dako::load_config(CfgReader& cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V1", V1);
    cfg.getDouble(secName, "Vy", Vy);
    cfg.getDouble(secName, "U1", U1);
    cfg.getDouble(secName, "U2", U2);

    cfg.getDouble(secName, "A1", A1);
    cfg.getDouble(secName, "A2", A2);
    cfg.getDouble(secName, "A3", A3);

    cfg.getDouble(secName, "K1", K1);
    cfg.getDouble(secName, "K2", K2);
    cfg.getDouble(secName, "K3", K3);
    cfg.getDouble(secName, "K4", K4);
    cfg.getDouble(secName, "K5", K5);

    cfg.getDouble(secName, "k1", k_1);
    cfg.getDouble(secName, "k2", k_2);
    cfg.getDouble(secName, "k3", k_3);
    cfg.getDouble(secName, "k4", k_4);
}
