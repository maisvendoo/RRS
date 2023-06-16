#include "tep70-switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70Switcher::TEP70Switcher(QObject* parent, int key_code, int kol_states)
    : Switcher(parent, key_code, kol_states)
    , old_state(state)
    , soundName("Switcher")
{
    setKolStates(kol_states);
    setKeyCode(key_code);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70Switcher::~TEP70Switcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70Switcher::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (old_state != state)
    {
        emit soundPlay(soundName);

        old_state = state;
    }
}
