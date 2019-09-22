#include "switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switcher::Switcher(QObject* parent) : Device(parent)
  , keyCode(0)
  , state(0)
  , kolStates(2)
  , p(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switcher::~Switcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::stepKeysControl(double t, double dt)
{
    if (getKeyState(keyCode))
    {
        if (p)
        {
            if(isShift())
                state++;
            else
                state--;

            state = cut(state, 0, kolStates - 1);

            p = false;
        }
    }
    else
        p = true;

    std::fill(is_switched.begin(), is_switched.end(), false);
    is_switched[static_cast<size_t>(state)] = true;
}


