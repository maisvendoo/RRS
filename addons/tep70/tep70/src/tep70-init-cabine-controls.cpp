#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initCabineControls()
{
    km = new ControllerKM2202();

    tumbler_field_weak1.setKolStates(3);
    tumbler_field_weak1.setKeyCode(KEY_3);
    tumbler_field_weak1.setState(1);

    tumbler_field_weak2.setKolStates(3);
    tumbler_field_weak2.setKeyCode(KEY_4);
    tumbler_field_weak2.setState(1);

    tumbler_water_zaluzi.setKolStates(3);
    tumbler_water_zaluzi.setKeyCode(KEY_5);
    tumbler_water_zaluzi.setState(1);

    tumbler_oil_zaluzi.setKolStates(3);
    tumbler_oil_zaluzi.setKeyCode(KEY_6);
    tumbler_oil_zaluzi.setState(1);
}
