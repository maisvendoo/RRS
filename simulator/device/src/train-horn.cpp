#include    "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainHorn::TrainHorn(QObject *parent) : Device(parent)
  , is_svistok(false)
  , is_tifon(false)
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

    bool is_svistok_old = is_svistok;
    is_svistok = getKeyState(KEY_Space);

    if (is_svistok_old != is_svistok)

    {
        if (is_svistok)
        {
            emit soundPlay("Svistok");
        }
        else
        {
            emit soundStop("Svistok");
        }
    }

    bool is_tifon_old = is_tifon;
    is_tifon = getKeyState(KEY_B);

    if (is_tifon_old != is_tifon)
    {
        if (is_tifon)
        {
            emit soundPlay("Tifon");
        }
        else
        {
            emit soundStop("Tifon");
        }
    }
}
