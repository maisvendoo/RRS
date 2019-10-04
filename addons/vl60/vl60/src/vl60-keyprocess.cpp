#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::keyProcess()
{
    if (autoStartTimer->isStarted())
        return;

    // Управление тумблером "Токоприемники"
    if (getKeyState(KEY_U))
    {
        if (isShift())
            pants_tumbler.set();
        else
            pants_tumbler.reset();
    }

    // Подъем/опускание переднего токоприемника
    if (getKeyState(KEY_I))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant1_tumbler.set();
        else
            pant1_tumbler.reset();

        // Задаем статус токоприемнику
        //pantographs[0]->setState(pants_tumbler.getState());
    }

    // Подъем/опускание заднего токоприемника
    if (getKeyState(KEY_O))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant2_tumbler.set();
        else
            pant2_tumbler.reset();

        // Задаем статус токоприемнику
        //pantographs[1]->setState(pants_tumbler.getState());
    }

    // Включение/выключение ГВ
    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_tumbler.set();
        else
            gv_tumbler.reset();
    }

    // Возврат защиты
    if (getKeyState(KEY_K))
        gv_return_tumbler.set();
    else
        gv_return_tumbler.reset();

    // Включение/выключение расщепителя фаз
    if (getKeyState(KEY_T))
    {
        if (isShift())
            fr_tumbler.set();
        else
            fr_tumbler.reset();
    }

    // Включение/выключение мотор-верниляторов

    // МВ1
    if (getKeyState(KEY_R))
    {
        if (isShift())
            mv_tumblers[MV1].set();
        else
            mv_tumblers[MV1].reset();
    }

    // МВ2
    if (getKeyState(KEY_F))
    {
        if (isShift())
            mv_tumblers[MV2].set();
        else
            mv_tumblers[MV2].reset();
    }

    // МВ3
    if (getKeyState(KEY_Y))
    {
        if (isShift())
            mv_tumblers[MV3].set();
        else
            mv_tumblers[MV3].reset();
    }

    // МВ4
    if (getKeyState(KEY_5))
    {
        if (isShift())
            mv_tumblers[MV4].set();
        else
            mv_tumblers[MV4].reset();
    }

    // МВ5
    if (getKeyState(KEY_6))
    {
        if (isShift())
            mv_tumblers[MV5].set();
        else
            mv_tumblers[MV5].reset();
    }

    // МВ6
    if (getKeyState(KEY_7))
    {
        if (isShift())
            mv_tumblers[MV6].set();
        else
            mv_tumblers[MV6].reset();
    }

    // Включение/выключение мотор-компрессора
    if (getKeyState(KEY_E))
    {
        if (isShift())
            mk_tumbler.set();
        else
            mk_tumbler.reset();
    }

    // Включение/выключение цепей управления
    if (getKeyState(KEY_J))
    {
        if (isShift())
            cu_tumbler.set();
        else
            cu_tumbler.reset();
    }

    // Нажатие РБ-1
    if (getKeyState(KEY_Z))
        rb[RB_1].set();
    else
        rb[RB_1].reset();

    // Нажатие РБС
    if (getKeyState(KEY_M))
        rb[RBS].set();
    else
        rb[RBS].reset();

    // Нажатие РБП
    if (getKeyState(KEY_Q))
        rb[RBP].set();
    else
        rb[RBP].reset();

    if (getKeyState(KEY_R))
    {
        if (isAlt() && !autoStartTimer->isStarted())
            autoStartTimer->start();
    }
}
