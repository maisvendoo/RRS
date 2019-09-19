#include "switcher.h"

Switcher::Switcher(QObject* parent) : Device(parent)
  , keyCode(0)
  , state(0)
  , kolStates(0)
  , p(false)
{

}

Switcher::~Switcher()
{

}

void Switcher::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    if (getKeyState(keyCode))
    {
        if (p)
        {
            if(isShift()&& state > 0)
                state -= 1;

            else if (state < kolStates - 1)
                state += 1;
            p = false;
        }
    }
    else
    {
        p = true;
    }
}


