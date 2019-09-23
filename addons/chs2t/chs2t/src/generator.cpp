#include "generator.h"
#include <QDir>

Generator::Generator(QObject* parent) : Device(parent)
  , Uf(0.0)
  , Lf(0.0)
  , Rf(0.0)
  , If(0.0)
  , omega(0.0)
  , E(0.0)
  , Ia(0.0)
  , La(0.0)
  , Ra(0.0)
  , Rt(0.0)
  , M(0.0)
  , Rgp(0.0)
  , Rdp(0.0)
{
}

Generator::~Generator()
{

}

void Generator::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    double s1 = calcCPhi(Y[0]);
    double s2 = s1 * omega;

    E = s2;

    M = s1 * Y[1];

    Ia = Y[1];

    Ut = Y[1] * Rt;

    If = Y[0];

    dYdt[0] = (Uf - Y[0] * Rf) / Lf;

    dYdt[1] = (s2 - Y[1] * (Ra + Rt)) / La;

}

void Generator::load_config(CfgReader& cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "R_a", Ra);
    cfg.getDouble(secName, "R_gp", Rgp);
    cfg.getDouble(secName, "R_dp", Rdp);

    Rf = Rgp + Rdp;

    QString cPhiFileName = "";

    cfg.getString(secName, "cPhi", cPhiFileName);

    cPhi.load((custom_config_dir + QDir::separator() + cPhiFileName).toStdString());
}

double Generator::calcCPhi(double I)
{
    return 1.2 * cPhi.getValue(I);
}
