#include    "automatic-train-stop.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AutoTrainStop::AutoTrainStop(QObject* parent) : BrakeDevice(parent)
{
    setKeySymbol(KEY_N);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setKeySymbol(std::uint16_t key_symbol)
{
    this->key_symbol = key_symbol;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::allowKey(bool allow)
{
    is_key_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isKeyAllowed() const
{
    return is_key_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::insertKey(bool insert)
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
bool AutoTrainStop::isKey() const
{
    return is_key.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setKeyOn(bool state)
{
    if (state)
    {
//        insertKey(true);

        if (isKey())
        {
            key_state.set();
        }
    }
    else
    {
        key_state.reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isKeyOn() const
{
    return key_state.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setPowered(bool powered)
{
    is_powered = powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isPowered() const
{
    return is_powered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setFLpressure(double pressure)
{
    pFL = pressure;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setBPpressure(double pressure)
{
    pBP = pressure;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getBPflow() const
{
    return QBP;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::setFlowAboveFailureValve(double flow)
{
    Qabove_failure_valve = flow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double AutoTrainStop::getPressureAboveFailureValve() const
{
    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::isWhistle() const
{
    return is_whistle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AutoTrainStop::getEmergencyBrakeContact() const
{
    return is_emergency_brake;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t AutoTrainStop::getSoundState(size_t idx) const
{
    if (idx == AUTOSTOP_WHISTLE)
    {
        return sound_state_t(is_whistle);
    }
    if (idx == BP_DRAIN_FLOW_SOUND)
    {
        // TODO //
        return sound_state_t(false);
    }
    if (idx >= 5)
    {
        return key_state.getSoundState(idx - 5);
    }
    return is_key.getSoundState(idx - 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float AutoTrainStop::getSoundSignal(size_t idx) const
{
    if (idx == AUTOSTOP_WHISTLE)
    {
        return sound_state_t::createSoundSignal(is_whistle);
    }
    if (idx == BP_DRAIN_FLOW_SOUND)
    {
        // TODO //
        return sound_state_t::createSoundSignal(false);
    }
    if (idx >= 5)
    {
        return key_state.getSoundSignal(idx - 5);
    }
    return is_key.getSoundSignal(idx - 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutoTrainStop::stepKeysControl(double t, double dt)
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
}
