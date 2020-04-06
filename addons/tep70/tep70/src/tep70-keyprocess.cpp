#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::keyProcess()
{
    button_disel_start = getKeyState(KEY_K);

    button_brake_release = getKeyState(KEY_R);

    button_svistok = getKeyState(KEY_Space);

    button_tifon = getKeyState(KEY_B);

    // Включение АЗВ "Управление общее"
    if (getKeyState(KEY_J))
    {
        if (isShift())
        {
            azv_common_control.set();
        }
        else
        {
            azv_common_control.reset();
        }
    }

    // Включение АЗВ "Управление тепловозом"
    if (getKeyState(KEY_U))
    {
        if (isShift())
        {
            azv_upr_tepl.set();
        }
        else
        {
            azv_upr_tepl.reset();
        }
    }

    // Включение АЗВ "Топливный насос"
    if (getKeyState(KEY_P))
    {
        if (isShift())
        {
            azv_fuel_pump.set();
        }
        else
        {
            azv_fuel_pump.reset();
        }
    }

    if (getKeyState(KEY_T))
    {
        if (isShift())
        {
            azv_edt_on.set();
        }
        else
        {
            azv_edt_on.reset();
        }
    }

    if (getKeyState(KEY_Y))
    {
        if (isShift())
        {
            azv_edt_power.set();
        }
        else
        {
            azv_edt_power.reset();
        }
    }

    if (getKeyState(KEY_V))
    {
        if (isShift())
        {
            azv_ept_on.set();
        }
        else
        {
            azv_ept_on.reset();
        }
    }

    if (getKeyState(KEY_1))
    {
        if (isShift())
        {
            tumbler_voltage.set();
        }
        else
        {
            tumbler_voltage.reset();
        }
    }

    if (getKeyState(KEY_2))
    {
        if (isShift())
        {
            tumbler_disel_stop.set();
        }
        else
        {
            tumbler_disel_stop.reset();
        }
    }

    if (getKeyState(KEY_7))
    {
        if (isShift())
        {
            azv_motor_compressor.set();
        }
        else
        {
            azv_motor_compressor.reset();
        }
    }
}
