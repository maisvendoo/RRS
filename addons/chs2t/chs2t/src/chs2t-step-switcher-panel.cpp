#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::setSwitcherState(Switcher *sw, signal_t signal)
{
    if (signal.is_active)
    {
        int pos = signal.cur_value - 1;

        emit soundPlay("tumbler");

        if (pos >= 0)
            sw->setState(pos);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void setTriggerState(Trigger &trig, signal_t signal)
{
    if (signal.is_active)
    {
        int pos = signal.cur_value - 1;

        if (pos >= 0)
        {
            if (static_cast<bool>(pos))
                trig.set();
            else
                trig.reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepSwitcherPanel()
{
    // Верхнаяя половина приборной панели
    setTriggerState(epb_switch, control_signals.analogSignal[SWP1_EPT]);
    setSwitcherState(mk_switcher[0], control_signals.analogSignal[SWP1_MK1]);
    setSwitcherState(fastSwitchSw, control_signals.analogSignal[SWP1_BV]);
    setSwitcherState(pantoSwitcher[0], control_signals.analogSignal[SWP1_TP1]);
    setSwitcherState(blindsSwitcher, control_signals.analogSignal[SWP1_VK]);
    setSwitcherState(pantoSwitcher[1], control_signals.analogSignal[SWP1_TP2]);

    // Нижняя половина панели
    setSwitcherState(mk_switcher[1], control_signals.analogSignal[SWP2_MK2]);
    setSwitcherState(motor_fan_switcher, control_signals.analogSignal[SWP2_MV]);
}
