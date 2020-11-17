#include    "tep70.h"


void TEP70::initAutostart()
{
    start_count = 0;

    triggers.push_back(&azv_common_control);
    triggers.push_back(&azv_fuel_pump);
    triggers.push_back(&button_start_disel);
    triggers.push_back(&azv_upr_tepl);
    triggers.push_back(&azv_ept_on);

    connect(&autoStartTimer, &Timer::process, this, &TEP70::slotAutostart);
    autoStartTimer.firstProcess(true);
    autoStartTimer.setTimeout(0.5);
}

void TEP70::stepAutostart(double t, double dt)
{
    autoStartTimer.step(t, dt);
}


void TEP70::slotAutostart()
{
    if (start_count < triggers.size())
    {
        triggers[start_count]->set();

        if (!kontaktor_oil_pump->getContactState(1) &&
             (triggers[start_count] == &button_start_disel) )
        {
            return;
        }
        else
        {
            button_start_disel.reset();

            if (!ru10->getContactState(1) && kontaktor_oil_pump->getContactState(1))
                return;
        }

        start_count++;
    }
    else
    {
        autoStartTimer.stop();
        tumbler_revers.setState(2);
    }
}
