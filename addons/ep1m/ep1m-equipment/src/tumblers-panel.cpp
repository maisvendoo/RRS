#include    "tumblers-panel.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1MTumblersPanel::EP1MTumblersPanel(QObject *parent) : Device(parent)
{
    initControl();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1MTumblersPanel::~EP1MTumblersPanel()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::setControl(std::set<std::uint16_t> *keys, control_signals_t *control_signals)
{
    Device::setControl(keys, control_signals);
    for (auto& tumbler : tumblers)
    {
        tumbler.setControl(pressed_keys);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::allowKey(bool allow)
{
    is_key_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1MTumblersPanel::isKeyAllowed() const
{
    return is_key_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::insertKey(bool insert)
{
    insert = insert && is_key_allowed;

    if (insert)
    {
        is_key.set();
    }
    else
    {
        if (!isKeyOn())
        {
            is_key.reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1MTumblersPanel::isKey() const
{
    return is_key.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::setKeyOn(bool state)
{
    if (state)
    {
        if (isKey())
        {
            key_state.set();
        }
    }
    else
    {
        if (isAllTumblersOff())
        {
            key_state.reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1MTumblersPanel::isKeyOn() const
{
    return key_state.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::setTumblerState(size_t tumbler_idx, bool state)
{
    state = state && isKeyOn();
    state ? tumblers[tumbler_idx].set() : tumblers[tumbler_idx].reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1MTumblersPanel::getTumblerState(size_t tumbler_index) const
{
    return tumblers[tumbler_index].getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float EP1MTumblersPanel::getKeyInsertSoundSignal(size_t idx)
{
    return is_key.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float EP1MTumblersPanel::getKeyTurnSoundSignal(size_t idx)
{
    return key_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float EP1MTumblersPanel::getTumblerSoundSignal(size_t tumbler_idx, size_t idx)
{
    return tumblers[tumbler_idx].getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::ode_system(const state_vector_t &Y,
                                   state_vector_t &dYdt,
                                   double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::step(double t, double dt)
{
    if (pressed_keys && getKeyState(*pressed_keys, key_symbol))
    {
        // Управляем новым нажатием на клавишу
        if (!prev_key)
        {
            prev_key = true; // Запоминаем, что клавиша нажата

            // Alt - вставляем/извлекаем ключ
            if (isModifier(*pressed_keys, MODIFIER_Alt))
            {
                insertKey(!isKey());
                return;
            }

            // Ctrl - отключаем ключ
            if (isModifier(*pressed_keys, MODIFIER_Control))
            {
                setKeyOn(false);
                return;
            }

            // Shift - включаем ключ
            if (isModifier(*pressed_keys, MODIFIER_Shift))
            {
                setKeyOn(true);
                return;
            }
        }
    }
    else
    {
        prev_key = false; // Запоминаем, что клавиша отпущена
    }

    if (isKeyOn())
    {
        // При разблокированной панели управляем тумблерами
        for (auto& tumbler : tumblers)
        {
            tumbler.step(t, dt);
        }
    }
    else
    {
        // При заблокированной панели все тумблеры выключены
        for (auto& tumbler : tumblers)
        {
            tumbler.reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1MTumblersPanel::initControl()
{
    key_symbol = KEY_Y;

    tumblers[TUMBLER_MSUD].setKeySymbolOn(KEY_9);
    tumblers[TUMBLER_MSUD].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_MSUD].setKeySymbolOff(KEY_9);
    tumblers[TUMBLER_MSUD].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_LOCK_VVK].setKeySymbolOn(KEY_0);
    tumblers[TUMBLER_LOCK_VVK].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_LOCK_VVK].setKeySymbolOff(KEY_0);
    tumblers[TUMBLER_LOCK_VVK].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_PANT1].setKeySymbolOn(KEY_I);
    tumblers[TUMBLER_PANT1].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_PANT1].setKeySymbolOff(KEY_I);
    tumblers[TUMBLER_PANT1].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_PANT2].setKeySymbolOn(KEY_O);
    tumblers[TUMBLER_PANT2].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_PANT2].setKeySymbolOff(KEY_O);
    tumblers[TUMBLER_PANT2].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_RETURN_PROTECTION].setKeySymbolOn(KEY_P);
    tumblers[TUMBLER_RETURN_PROTECTION].setKeyModifierOn(MODIFIER_OnlyAlt);
    tumblers[TUMBLER_RETURN_PROTECTION].setKeySymbolOff(KEY_Undefined);
    tumblers[TUMBLER_RETURN_PROTECTION].setKeyModifierOff(KEY_Undefined);

    tumblers[TUMBLER_MAIN_SWITCH].setKeySymbolOn(KEY_P);
    tumblers[TUMBLER_MAIN_SWITCH].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_MAIN_SWITCH].setKeySymbolOff(KEY_P);
    tumblers[TUMBLER_MAIN_SWITCH].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_AUX_MACHINES].setKeySymbolOn(KEY_T);
    tumblers[TUMBLER_AUX_MACHINES].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_AUX_MACHINES].setKeySymbolOff(KEY_T);
    tumblers[TUMBLER_AUX_MACHINES].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_COMPRESSOR].setKeySymbolOn(KEY_4);
    tumblers[TUMBLER_COMPRESSOR].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_COMPRESSOR].setKeySymbolOff(KEY_4);
    tumblers[TUMBLER_COMPRESSOR].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_MOTOR_FAN1].setKeySymbolOn(KEY_1);
    tumblers[TUMBLER_MOTOR_FAN1].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_MOTOR_FAN1].setKeySymbolOff(KEY_1);
    tumblers[TUMBLER_MOTOR_FAN1].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_MOTOR_FAN2].setKeySymbolOn(KEY_2);
    tumblers[TUMBLER_MOTOR_FAN2].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_MOTOR_FAN2].setKeySymbolOff(KEY_2);
    tumblers[TUMBLER_MOTOR_FAN2].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_MOTOR_FAN3].setKeySymbolOn(KEY_3);
    tumblers[TUMBLER_MOTOR_FAN3].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_MOTOR_FAN3].setKeySymbolOff(KEY_3);
    tumblers[TUMBLER_MOTOR_FAN3].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[TUMBLER_EPT].setKeySymbolOn(KEY_V);
    tumblers[TUMBLER_EPT].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[TUMBLER_EPT].setKeySymbolOff(KEY_V);
    tumblers[TUMBLER_EPT].setKeyModifierOff(MODIFIER_OnlyControl);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1MTumblersPanel::isAllTumblersOff() const
{
    for (const auto& tumbler : tumblers)
    {
        if (tumbler.getState())
        {
            return false;
        }
    }

    return true;
}
