#include    "shield-229.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Shield_229::Shield_229(QObject* parent) : Device(parent)
{
    initControl();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_229::setControl(std::set<std::uint16_t> *keys, control_signals_t *control_signals)
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
void Shield_229::setTumblerState(size_t tumbler_idx, bool state)
{
    state ? tumblers[tumbler_idx].set() : tumblers[tumbler_idx].reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Shield_229::getTumblerState(size_t tumbler_index) const
{
    return tumblers[tumbler_index].getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Shield_229::getTumblerSoundSignal(size_t tumbler_idx, size_t idx)
{
    return tumblers[tumbler_idx].getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_229::ode_system(const state_vector_t &Y,
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
void Shield_229::step(double t, double dt)
{
    // Управляем неблокируемыми тумблерами
    for (auto& tumbler : tumblers)
    {
        tumbler.step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Shield_229::initControl()
{
    tumblers[TIFON].setInitState(false);

    tumblers[WHISTLE].setInitState(false);

    tumblers[CAB_HEAT].setInitState(false);

    tumblers[CAB_LIGHT_LOW].setKeySymbolOn(KEY_K);
    tumblers[CAB_LIGHT_LOW].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[CAB_LIGHT_LOW].setKeySymbolOff(KEY_K);
    tumblers[CAB_LIGHT_LOW].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[CAB_LIGHT_HIGH].setKeySymbolOn(KEY_K);
    tumblers[CAB_LIGHT_HIGH].setKeyModifierOn(MODIFIER_OnlyAlt);
    tumblers[CAB_LIGHT_HIGH].setKeySymbolOff(KEY_K);
    tumblers[CAB_LIGHT_HIGH].setKeyModifierOff(MODIFIER_OnlyAlt);

    tumblers[RESERV_1].setInitState(false);

    tumblers[SHASSIS_LIGHT].setInitState(false);

    tumblers[DEVICES_LIGHT].setKeySymbolOn(KEY_L);
    tumblers[DEVICES_LIGHT].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[DEVICES_LIGHT].setKeySymbolOff(KEY_L);
    tumblers[DEVICES_LIGHT].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[BUFFERLIGHT_L].setKeySymbolOn(KEY_G);
    tumblers[BUFFERLIGHT_L].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[BUFFERLIGHT_L].setKeySymbolOff(KEY_G);
    tumblers[BUFFERLIGHT_L].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[BUFFERLIGHT_R].setKeySymbolOn(KEY_J);
    tumblers[BUFFERLIGHT_R].setKeyModifierOn(MODIFIER_OnlyShift);
    tumblers[BUFFERLIGHT_R].setKeySymbolOff(KEY_J);
    tumblers[BUFFERLIGHT_R].setKeyModifierOff(MODIFIER_OnlyControl);

    tumblers[RESERV_2].setInitState(false);

    tumblers[ALSN_CHECK].setInitState(false);
}
