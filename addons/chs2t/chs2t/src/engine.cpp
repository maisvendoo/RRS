#include "engine.h"

#include    <QDir>

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Engine::Engine(QObject* parent) : Device(parent)
  , poz(0)
  , n(0)
  , beta(1)
  , R_a(0.032)
  , R_gp(0.0232)
  , R_dp(0.0127)
  , R_r(0)
  , L_af(0.001)
  , omega(0.0)
  , torque(0.0)
  , U(0.0)
  , omega_nom(73.8)
  , direction(1)

{

}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
Engine::~Engine()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::setBetaStep(int step)
{
    if (fieldStep.contains(step))
    {
        setBeta(fieldStep[step]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::setDirection(int revers_state)
{
    if (revers_state == 0)
        return;
    else
        direction = revers_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Engine::getI12() const
{
    return getIa() * static_cast<double>(amp_state.is12on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Engine::getI34() const
{
    return getIa() * static_cast<double>(amp_state.is34on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Engine::getI56() const
{
    return getIa() * static_cast<double>(amp_state.is56on);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)

    double R;
    R = R_r / n + R_a + R_gp * beta + R_dp;
    dYdt[0] = (U / n - R * Y[0] - calcCPhi(Y[0] * beta * direction) * omega) / L_af;


}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::load_config(CfgReader& cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "R_a", R_a);
    cfg.getDouble(secName, "R_gp", R_gp);
    cfg.getDouble(secName, "R_dp", R_dp);
    cfg.getDouble(secName, "R_r", R_r);
    cfg.getDouble(secName, "L_af", L_af);
    cfg.getDouble(secName, "OmegaNom", omega_nom);

    QString cPhiFileName = "";

    cfg.getString(secName, "cPhi", cPhiFileName);

    cPhi.load((custom_config_dir + QDir::separator() + cPhiFileName).toStdString());

    QDomNode secNode;

    secNode = cfg.getFirstSection("FieldPos");

    while (!secNode.isNull())
    {
        double field_step = 1.0;
        int number = 0;

        cfg.getInt(secNode, "Number", number);
        cfg.getDouble(secNode, "beta", field_step);

        fieldStep.insert(number, field_step);

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::preStep(state_vector_t& Y, double t)
{
    Q_UNUSED(t)

    torque = calcCPhi(Y[0] * beta * direction) * Y[0];

    if (poz < 21)
        n = 6;
    else
    {
        if (poz < 34)
            n = 3;
        else
            n = 2;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Engine::calcCPhi(double I)
{
    return 1.2 * cPhi.getValue(I);
}
