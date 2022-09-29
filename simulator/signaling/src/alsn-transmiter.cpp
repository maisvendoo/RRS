#include    "alsn-transmiter.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Transmiter::Transmiter(QObject *parent) : Device(parent)
  , is_red(false)
  , is_green(false)
  , is_yellow(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Transmiter::~Transmiter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Transmiter::setState(bool is_red, bool is_green, bool is_yellow)
{
    if (this->is_red != is_red)
    {
        this->is_red = is_red;

        if (this->is_red)
        {
            emit sendAlsnCode(RED_ALSN);
            return;
        }
    }

    if (this->is_green != is_green)
    {
        this->is_green = is_green;

        if (this->is_green)
        {
            emit sendAlsnCode(GREEN_ALSN);
            return;
        }
    }

    if (this->is_yellow != is_yellow)
    {
        this->is_yellow = is_yellow;

        if (this->is_yellow)
        {
            emit sendAlsnCode(YELLOW_ALSN);
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Transmiter::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Transmiter::ode_system(const state_vector_t &Y,
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
void Transmiter::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
