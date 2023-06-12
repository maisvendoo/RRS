#include "chs2t-switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CHS2TSwitcher::CHS2TSwitcher(QObject* parent, int key_code, int kol_states)
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
CHS2TSwitcher::~CHS2TSwitcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2TSwitcher::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y);
    Q_UNUSED(t);

    QMap<int, int>::const_iterator springs_i = springStates.constBegin();

    while (springs_i != springStates.constEnd()) {
        if (springs_i.key() == state && !getKeyState(keyCode))
        {
            state = springs_i.value();
        }
        springs_i++;
    }

    if (old_state != state)
    {
        emit soundPlay(soundName);

        old_state = state;
    }
}
