#include    "vl60pk.h"

#include    "key-symbols.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::keyProcess()
{
    if (autoStartTimer->isStarted())
        return;

    if (getKeyState(KEY_R) && isAlt())
    {
        initTriggers();
        autoStartTimer->start();
        return;
    }

    // Управление тумблером "Токоприемники"
    if (getKeyState(KEY_U))
    {
        if (isShift())
            pants_tumbler[cabine_idx].set();
        else
            pants_tumbler[cabine_idx].reset();
    }

    // Подъем/опускание переднего токоприемника
    if (getKeyState(KEY_I))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant1_tumbler[cabine_idx].set();
        else
            pant1_tumbler[cabine_idx].reset();
    }

    // Подъем/опускание заднего токоприемника
    if (getKeyState(KEY_O))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant2_tumbler[cabine_idx].set();
        else
            pant2_tumbler[cabine_idx].reset();

    }

    // Включение/выключение ГВ
    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_tumbler[cabine_idx].set();
        else
            gv_tumbler[cabine_idx].reset();
    }

    // Возврат защиты
    if (getKeyState(KEY_K))
        gv_return_tumbler[cabine_idx].set();
    else
        gv_return_tumbler[cabine_idx].reset();

    // Включение/выключение расщепителя фаз
    if (getKeyState(KEY_T))
    {
        if (isShift())
            fr_tumbler[cabine_idx].set();
        else
            fr_tumbler[cabine_idx].reset();
    }

    // Включение/выключение мотор-вентиляторов
    // МВ1
    if (getKeyState(KEY_R))
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV1].set();
        else
            mv_tumblers[cabine_idx][MV1].reset();
    }

    // МВ2
    if (getKeyState(KEY_F))
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV2].set();
        else
            mv_tumblers[cabine_idx][MV2].reset();
    }

    // МВ3
    if (getKeyState(KEY_Y))
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV3].set();
        else
            mv_tumblers[cabine_idx][MV3].reset();
    }

    // МВ4
    if (getKeyState(KEY_5) && !isAlt())
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV4].set();
        else
            mv_tumblers[cabine_idx][MV4].reset();
    }

    // МВ5
    if (getKeyState(KEY_6) && !isAlt())
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV5].set();
        else
            mv_tumblers[cabine_idx][MV5].reset();
    }

    // МВ6
    if (getKeyState(KEY_7) && !isAlt())
    {
        if (isShift())
            mv_tumblers[cabine_idx][MV6].set();
        else
            mv_tumblers[cabine_idx][MV6].reset();
    }

    // Включение/выключение мотор-компрессора
    if (getKeyState(KEY_E))
    {
        if (isShift())
            mk_tumbler[cabine_idx].set();
        else
            mk_tumbler[cabine_idx].reset();
    }

    // Включение/выключение цепей управления
    if (getKeyState(KEY_J))
    {
        if (isShift())
            cu_tumbler[cabine_idx].set();
        else
            cu_tumbler[cabine_idx].reset();
    }

    // Нажатие РБ-1
    if (getKeyState(KEY_Z))
        rb[cabine_idx][RB_1].set();
    else
        rb[cabine_idx][RB_1].reset();

    // Нажатие РБС
    // Если активна РБС на внешнем пульте
    if (control_signals.analogSignal[CS_RBS].is_active)
    {
        // реагируем на состояние РБС на внешнем пульте
        if (static_cast<bool>(control_signals.analogSignal[CS_RBS].cur_value))
            rb[cabine_idx][RBS].set();
        else
            rb[cabine_idx][RBS].reset();
    }
    else // иначе
    {
        // обрабатываем клавиши
        if (getKeyState(KEY_M))
            rb[cabine_idx][RBS].set();
        else
            rb[cabine_idx][RBS].reset();
    }

    // Нажатие РБП
    if (getKeyState(KEY_Tilde))
        rb[cabine_idx][RBP].set();
    else
        rb[cabine_idx][RBP].reset();

    // Включение/выключение ЭПK
    if (getKeyState(KEY_N))
    {
        if (isShift())
            key_epk[cabine_idx].set();
        else
            key_epk[cabine_idx].reset();
    }

    // Включение/выключение ЭПТ
    if (getKeyState(KEY_V))
    {
        if (isShift())
            epb_switch[cabine_idx].set();
        else
            epb_switch[cabine_idx].reset();
    }
}
