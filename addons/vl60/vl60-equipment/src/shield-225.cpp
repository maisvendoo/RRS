#include    "shield-225.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Shield_225::Shield_225(QObject* parent) : Device(parent)
{
    initControl();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::setControl(std::set<std::uint16_t> *keys, control_signals_t *control_signals)
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
void Shield_225::allowKey(bool allow)
{
    is_key_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_225::isKeyAllowed() const
{
    return is_key_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::insertKey(bool insert)
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
bool Shield_225::isKey() const
{
    return is_key.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::setKeyOn(bool state)
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
bool Shield_225::isKeyOn() const
{
    return key_state.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::setTumblerState(size_t tumbler_idx, bool state)
{
    if (!isKeyOn())
    {
        for (const auto& id : locked_tumblers)
        {
            if (tumbler_idx == id)
            {
                tumblers[tumbler_idx].reset();
                return;
            }
        }
    }

    state ? tumblers[tumbler_idx].set() : tumblers[tumbler_idx].reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_225::getTumblerState(size_t tumbler_index) const
{
    return tumblers[tumbler_index].getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_225::getKeyInsertSoundSignal(size_t idx)
{
    return is_key.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_225::getKeyTurnSoundSignal(size_t idx)
{
    return key_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_225::getTumblerSoundSignal(size_t tumbler_idx, size_t idx)
{
    return tumblers[tumbler_idx].getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::ode_system(const state_vector_t &Y,
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
void Shield_225::step(double t, double dt)
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
/*
    // Управляем неблокируемыми тумблерами
    for (const auto& id : no_locked_tumblers)
    {
        tumblers[id].step(t, dt);
    }
*/
    for (const auto& id : locked_tumblers)
    {
        if (isKeyOn())
        {
            // При разблокированной панели управляем тумблерами
            tumblers[id].step(t, dt);
        }
        else
        {
            // При заблокированной панели все тумблеры выключены
            tumblers[id].reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_225::initControl()
{
    key_symbol = KEY_Undefined;

    tumblers[AUTOSAND].setInitState(false);

    tumblers[FAN_6].setKeySymbolOn(KEY_6);
    tumblers[FAN_6].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_6].setKeySymbolOff(KEY_6);
    tumblers[FAN_6].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[FAN_5].setKeySymbolOn(KEY_5);
    tumblers[FAN_5].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_5].setKeySymbolOff(KEY_5);
    tumblers[FAN_5].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[FAN_4].setKeySymbolOn(KEY_4);
    tumblers[FAN_4].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_4].setKeySymbolOff(KEY_4);
    tumblers[FAN_4].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[FAN_3].setKeySymbolOn(KEY_3);
    tumblers[FAN_3].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_3].setKeySymbolOff(KEY_3);
    tumblers[FAN_3].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[FAN_2].setKeySymbolOn(KEY_2);
    tumblers[FAN_2].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_2].setKeySymbolOff(KEY_2);
    tumblers[FAN_2].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[FAN_1].setKeySymbolOn(KEY_1);
    tumblers[FAN_1].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[FAN_1].setKeySymbolOff(KEY_1);
    tumblers[FAN_1].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[COMPRESSOR].setKeySymbolOn(KEY_7);
    tumblers[COMPRESSOR].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[COMPRESSOR].setKeySymbolOff(KEY_7);
    tumblers[COMPRESSOR].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[PHASE_SPLITTER].setKeySymbolOn(KEY_T);
    tumblers[PHASE_SPLITTER].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[PHASE_SPLITTER].setKeySymbolOff(KEY_T);
    tumblers[PHASE_SPLITTER].setKeyModifierOff(MODIFIER_OnlyControl);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_225::isAllTumblersOff() const
{
    for (const auto& id : locked_tumblers)
    {
        if (tumblers[id].getState())
        {
            return false;
        }
    }

    return true;
}
