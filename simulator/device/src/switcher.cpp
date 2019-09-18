#include "switcher.h"

Switcher::Switcher(QObject* parent) : Device(parent)
  , keyCode(0)
  , state(0)
{

}

Switcher::~Switcher()
{

}

void Switcher::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    if (getKeyState(keyCode))
    {
        if(isShift()&& state > 0)
            state -= 1;
        if (state < 3)
            state += 1;
    }
}


