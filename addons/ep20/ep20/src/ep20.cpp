#include    "ep20.h"

#include    <CfgReader.h>
#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP20::EP20()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP20::~EP20()
{

}

void EP20::initialization()
{

}

void EP20::initHighVoltageScheme()
{
    for (size_t i = 0; i < pantograph.size(); ++i)
    {
        pantograph[i] = new Pantograph();
        pantograph[i]->read_custom_config(config_dir + QDir::separator() + "pantograph");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP20::step(double t, double dt)
{

}

void EP20::stepHighVoltageScheme(double t, double dt)
{

    for (auto pant : pantograph)
    {
        pant->setUks(25000.0);
        pant->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP20::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {

    }
}

GET_VEHICLE(EP20)
