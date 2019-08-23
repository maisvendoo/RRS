//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------

#include "engine.h"

#include    <QDir>

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Engine::Engine(QObject* parent) : Device(parent)
  , n(0)
{

}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
Engine::~Engine()
{

}

void Engine::setBetaStep(int step)
{

}

void Engine::setDirection(int revers_state)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

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
        double field_step = 0.95;
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

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Engine::stepKeysControl(double t, double dt)
{
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
    return cPhi.getValue(I);
}
