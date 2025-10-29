#include    "shield-223.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Shield_223::Shield_223(QObject* parent) : Device(parent)
{
    initControl();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_223::setControl(std::set<std::uint16_t> *keys, control_signals_t *control_signals)
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
void Shield_223::allowKey(bool allow)
{
    is_key_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_223::isKeyAllowed() const
{
    return is_key_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_223::insertKey(bool insert)
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
bool Shield_223::isKey() const
{
    return is_key.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_223::setKeyOn(bool state)
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
bool Shield_223::isKeyOn() const
{
    return key_state.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_223::setTumblerState(size_t tumbler_idx, bool state)
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
bool Shield_223::getTumblerState(size_t tumbler_index) const
{
    return tumblers[tumbler_index].getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_223::getKeyInsertSoundSignal(size_t idx)
{
    return is_key.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_223::getKeyTurnSoundSignal(size_t idx)
{
    return key_state.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_223::getTumblerSoundSignal(size_t tumbler_idx, size_t idx)
{
    return tumblers[tumbler_idx].getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_223::ode_system(const state_vector_t &Y,
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
void Shield_223::step(double t, double dt)
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

    // Управляем неблокируемыми тумблерами
    for (const auto& id : no_locked_tumblers)
    {
        tumblers[id].step(t, dt);
    }

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
void Shield_223::initControl()
{
    key_symbol = KEY_Undefined;

    tumblers[SPOTLIGHT_HIGH].setKeySymbolOn(KEY_H);
    tumblers[SPOTLIGHT_HIGH].setKeyModifierOn(MODIFIER_OnlyAlt);
    tumblers[SPOTLIGHT_HIGH].setKeySymbolOff(KEY_H);
    tumblers[SPOTLIGHT_HIGH].setKeyModifierOff(MODIFIER_OnlyAlt);

    tumblers[SPOTLIGHT_LOW].setKeySymbolOn(KEY_H);
    tumblers[SPOTLIGHT_LOW].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[SPOTLIGHT_LOW].setKeySymbolOff(KEY_H);
    tumblers[SPOTLIGHT_LOW].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[RADIO].setInitState(false);

    tumblers[CIRCUIT].setKeySymbolOn(KEY_Y);
    tumblers[CIRCUIT].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[CIRCUIT].setKeySymbolOff(KEY_Y);
    tumblers[CIRCUIT].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[PANT_BWD].setKeySymbolOn(KEY_O);
    tumblers[PANT_BWD].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[PANT_BWD].setKeySymbolOff(KEY_O);
    tumblers[PANT_BWD].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[PANT_FWD].setKeySymbolOn(KEY_I);
    tumblers[PANT_FWD].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[PANT_FWD].setKeySymbolOff(KEY_I);
    tumblers[PANT_FWD].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[PANTS].setKeySymbolOn(KEY_U);
    tumblers[PANTS].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[PANTS].setKeySymbolOff(KEY_U);
    tumblers[PANTS].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[RETURN_PROTECTION].setKeySymbolOn(KEY_P);
    tumblers[RETURN_PROTECTION].setKeyModifierOn(MODIFIER_OnlyAlt);
    tumblers[RETURN_PROTECTION].setKeySymbolOff(KEY_Undefined);
    tumblers[RETURN_PROTECTION].setKeyModifierOff(KEY_Undefined);

    tumblers[MAIN_SWITCH].setKeySymbolOn(KEY_P);
    tumblers[MAIN_SWITCH].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[MAIN_SWITCH].setKeySymbolOff(KEY_P);
    tumblers[MAIN_SWITCH].setKeyModifierOff(MODIFIER_OnlyControl);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_223::isAllTumblersOff() const
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
