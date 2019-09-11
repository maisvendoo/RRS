#include "dako.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Dako::Dako(QObject* parent) : Device(parent)
  , V1(0.0005)
  , Vy(0.008)
  , U1(13.9)
  , U2(22.2)
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
    int s1 = static_cast<int>(pf((U - U2) * k1));
    double s2 = cut(s1, 0, 1);

    int s3 = static_cast<int>(nf((U - U1) * k2));
    double s4 = cut(s3, 0, 1);

    double s5 = Y[0] * A1 + Y[1] * A2 - ptc * A3;

    int s6 = static_cast<int>(pf(s5));
    double s7 = cut(s6, 0, 1) * U2;

    int s8 = static_cast<int>(nf(s5));
    double s9 = cut(s8, 0, 1) * U1;

    Qtc = (pgr - ptc) * K4 * s7 - ptc * K3 * s9;

    dYdt[0] = Qvr / Vy;
    py = Y[0];

    dYdt[1] = (Q1 * s2 - Y[1] * K2 * s4) / V1;
    p1 = Y[1];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Dako::load_config(CfgReader& cfg)
{
    cfg.getDouble("Dako", "A1", A1);
    cfg.getDouble("Dako", "A2", A2);
    cfg.getDouble("Dako", "A3", A3);

    cfg.getDouble("Dako", "K1", K1);
    cfg.getDouble("Dako", "K2", K2);
    cfg.getDouble("Dako", "K3", K3);
    cfg.getDouble("Dako", "K4", K4);

    cfg.getDouble("Dako", "k1", k1);
    cfg.getDouble("Dako", "k2", k2);
}
