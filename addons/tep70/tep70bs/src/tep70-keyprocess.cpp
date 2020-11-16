#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::keyProcess()
{
    if (autoStartTimer.isStarted())
        return;

    if (getKeyState(KEY_K))
        button_start_disel.set();
    else
        button_start_disel.reset();

    button_brake_release = !getKeyState(KEY_Y);

    button_svistok = getKeyState(KEY_Space);

    button_tifon = getKeyState(KEY_B);

    button_RB1 = getKeyState(KEY_Z);

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

    if (getKeyState(KEY_N))
    {
        if (isShift())
        {
            epk_key.set();
        }
        else
        {
            epk_key.reset();
        }
    }

    bool is_svistok_old = is_svistok;
    is_svistok = getKeyState(KEY_Space);

    if (is_svistok_old != is_svistok)

    {
        if (is_svistok)
        {
            emit soundPlay("Svistok");
        }
        else
        {
            emit soundStop("Svistok");
        }
    }

    bool is_tifon_old = is_tifon;
    is_tifon = getKeyState(KEY_B);

    if (is_tifon_old != is_tifon)
    {
        if (is_tifon)
        {
            emit soundPlay("Tifon");
        }
        else
        {
            emit soundStop("Tifon");
        }
    }

    if (getKeyState(KEY_R))
    {
        if (isAlt() && !autoStartTimer.isStarted())
            autoStartTimer.start();
    }
}
