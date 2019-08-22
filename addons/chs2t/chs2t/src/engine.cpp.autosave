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

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Engine::Engine(QObject* parent) : Device(parent)
  , n(0)
{

}

Engine::~Engine()
{

}

void Engine::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void Engine::load_config(CfgReader& cfg)
{

}

void Engine::preStep(state_vector_t& Y, double t)
{

}

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
