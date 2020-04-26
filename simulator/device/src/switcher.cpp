#include "switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switcher::Switcher(QObject* parent, int key_code, int kol_states) : Device(parent)
  , keyCode(0)
  , state(0)
  , kolStates(0)
  , ableToPress(false)
  , PlusSoundName("plus")
  , MinusSoundName("minus")
{
    setKolStates(kol_states);
    setKeyCode(key_code);
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
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (getKeyState(keyCode))
    {
        if (ableToPress)
        {
            int old_state = state;

            if(isShift())
            {
                state++;
            }

            else
            {
                state--;
            }

            state = cut(state, 0, kolStates - 1);

            if (state < old_state)
                emit soundPlay(MinusSoundName);

            if (state > old_state)
                emit soundPlay(PlusSoundName);


            ableToPress = false;
        }
    }

    else
    {
        ableToPress = true;
    }

    std::fill(is_switched.begin(), is_switched.end(), false);
    is_switched[static_cast<size_t>(state)] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::setPlusSoundName(QString soundName)
{
    PlusSoundName = soundName;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::setMinusSoundName(QString soundName)
{
    MinusSoundName = soundName;
}



