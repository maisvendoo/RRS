#include "chs2t.h"

void CHS2T::stepSwitcherPanel()
{
    if (control_signals.analogSignal[SWP1_BV].is_active)
        fastSwitchSw->setState(control_signals.analogSignal[SWP1_BV].cur_value);

}
