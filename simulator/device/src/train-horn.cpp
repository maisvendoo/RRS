#include    "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::TrainHorn(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::~TrainHorn()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::ode_system(const state_vector_t &Y,
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
void TrainHorn::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainHorn::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (getKeyState(KEY_Space))
    {
        emit soundSetVolume("Svistok", 100);
    }
    else
    {
        emit soundSetVolume("Svistok", 0);
    }

    if (getKeyState(KEY_B))
    {
        emit soundSetVolume("Tifon", 100);
    }
    else
    {
        emit soundSetVolume("Tifon", 0);
    }
}
