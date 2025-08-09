#include    "vl60pk.h"

#include    "key-symbols.h"
#include    "timer.h"

#include "coupling-operating-rod.h"
#include "brake-crane.h"
#include "brake-lock.h"
#include "loco-crane.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose-epb.h"
#include "sanding-system.h"
#include "train-horn.h"

#include "kme-60-044.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::keyProcess()
{
    if (needDebugMsg)
        debugPrint();

    // Сцепные устройства
    oper_rod_fwd->setControl(&pressed_keys);
    oper_rod_bwd->setControl(&pressed_keys);

    // Концевые краны и рукава тормозной магистрали
    anglecock_bp_fwd->setControl(&pressed_keys);
    anglecock_bp_bwd->setControl(&pressed_keys);
    hose_bp_fwd->setControl(&pressed_keys);
    hose_bp_bwd->setControl(&pressed_keys);

    // Концевые краны и рукава питательной магистрали
    anglecock_fl_fwd->setControl(&pressed_keys);
    anglecock_fl_bwd->setControl(&pressed_keys);
    hose_fl_fwd->setControl(&pressed_keys);
    hose_fl_bwd->setControl(&pressed_keys);

    // Концевые краны и рукава магистрали тормозных цилиндров
    anglecock_bc_fwd->setControl(&pressed_keys);
    anglecock_bc_bwd->setControl(&pressed_keys);
    hose_bc_fwd->setControl(&pressed_keys);
    hose_bc_bwd->setControl(&pressed_keys);

    // Песочница
    sand_system->setControl(&pressed_keys);

    // Управление тормозным оборудованием в кабинах
    for (auto cabine_idx : {CAB1, CAB2})
    {
        // Управляем блокировкой тормозов
        brake_lock[cabine_idx]->setControl(&pressed_keys_by_cabine[cabine_idx]);

        // Управляем краном, учитывая возможное наличие внешнего пульта
        // TODO // перенести freejoy во вьювер, его команды передавать по сети,
        // TODO // и также указывая индекс кабины
        if (control_signals.analogSignal[CS_BRAKE_CRANE].is_active)
        {
            int brake_crane_pos = static_cast<int>(control_signals.analogSignal[CS_BRAKE_CRANE].cur_value);
            brake_crane[cabine_idx]->setHandlePosition(brake_crane_pos);
        }
        else
        {
            brake_crane[cabine_idx]->setControl(&pressed_keys_by_cabine[cabine_idx]);
        }

        // Управляем краном, учитывая возможное наличие внешнего пульта
        // TODO // перенести freejoy во вьювер, его команды передавать по сети,
        // TODO // и также указывая индекс кабины
        if (control_signals.analogSignal[CS_LOCO_CRANE].is_active)
        {
            double pos = 0.0;

            if (static_cast<bool>(control_signals.analogSignal[CS_RELEASE_VALVE].cur_value))
            {
                loco_crane[cabine_idx]->release(true);
                pos = -1.0;
            }
            else
            {
                loco_crane[cabine_idx]->release(false);
                pos = control_signals.analogSignal[CS_LOCO_CRANE].cur_value;
            }

            loco_crane[cabine_idx]->setHandlePosition(pos);
        }
        else
        {
            loco_crane[cabine_idx]->setControl(&pressed_keys_by_cabine[cabine_idx]);
        }

        // Тифон и свисток
        horn[cabine_idx]->setControl(&pressed_keys_by_cabine[cabine_idx]);
    }

    // На пульты в кабинах отдельный цикл, с проверкой на программу автозапуска
    for (auto cabine_idx : {CAB1, CAB2})
    {
        if (autoStartTimer->isStarted())
            continue;

        if (getKeyState(KEY_R, cabine_idx) && isAlt(cabine_idx))
        {
            initTriggers(cabine_idx);
            autoStartTimer->start();
            continue;
        }

        // Контроллер машиниста
        controller[cabine_idx]->setControl(&pressed_keys_by_cabine[cabine_idx]);

        // Управление тумблером "Токоприемники"
        if (getKeyState(KEY_U, cabine_idx))
        {
            if (isShift())
                pants_tumbler[cabine_idx].set();
            else
                pants_tumbler[cabine_idx].reset();
        }

        // Подъем/опускание переднего токоприемника
        if (getKeyState(KEY_I, cabine_idx))
        {
            // Переводим тумблер в нужное фиксированное положение
            if (isShift())
                pant1_tumbler[cabine_idx].set();
            else
                pant1_tumbler[cabine_idx].reset();
        }

        // Подъем/опускание заднего токоприемника
        if (getKeyState(KEY_O, cabine_idx))
        {
            // Переводим тумблер в нужное фиксированное положение
            if (isShift())
                pant2_tumbler[cabine_idx].set();
            else
                pant2_tumbler[cabine_idx].reset();

        }

        // Включение/выключение ГВ
        if (getKeyState(KEY_P, cabine_idx))
        {
            if (isShift())
                gv_tumbler[cabine_idx].set();
            else
                gv_tumbler[cabine_idx].reset();
        }

        // Возврат защиты
        if (getKeyState(KEY_K, cabine_idx))
            gv_return_tumbler[cabine_idx].set();
        else
            gv_return_tumbler[cabine_idx].reset();

        // Включение/выключение расщепителя фаз
        if (getKeyState(KEY_T, cabine_idx))
        {
            if (isShift())
                fr_tumbler[cabine_idx].set();
            else
                fr_tumbler[cabine_idx].reset();
        }

        // Включение/выключение мотор-вентиляторов
        // МВ1
        if (getKeyState(KEY_R, cabine_idx))
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV1].set();
            else
                mv_tumblers[cabine_idx][MV1].reset();
        }

        // МВ2
        if (getKeyState(KEY_F, cabine_idx))
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV2].set();
            else
                mv_tumblers[cabine_idx][MV2].reset();
        }

        // МВ3
        if (getKeyState(KEY_Y, cabine_idx))
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV3].set();
            else
                mv_tumblers[cabine_idx][MV3].reset();
        }

        // МВ4
        if (getKeyState(KEY_5, cabine_idx) && !isAlt())
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV4].set();
            else
                mv_tumblers[cabine_idx][MV4].reset();
        }

        // МВ5
        if (getKeyState(KEY_6, cabine_idx) && !isAlt())
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV5].set();
            else
                mv_tumblers[cabine_idx][MV5].reset();
        }

        // МВ6
        if (getKeyState(KEY_7, cabine_idx) && !isAlt())
        {
            if (isShift())
                mv_tumblers[cabine_idx][MV6].set();
            else
                mv_tumblers[cabine_idx][MV6].reset();
        }

        // Включение/выключение мотор-компрессора
        if (getKeyState(KEY_E, cabine_idx))
        {
            if (isShift())
                mk_tumbler[cabine_idx].set();
            else
                mk_tumbler[cabine_idx].reset();
        }

        // Включение/выключение цепей управления
        if (getKeyState(KEY_J, cabine_idx))
        {
            if (isShift())
                cu_tumbler[cabine_idx].set();
            else
                cu_tumbler[cabine_idx].reset();
        }

        // Нажатие РБ-1
        if (getKeyState(KEY_Z, cabine_idx))
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
            if (getKeyState(KEY_M, cabine_idx))
                rb[cabine_idx][RBS].set();
            else
                rb[cabine_idx][RBS].reset();
        }

        // Нажатие РБП
        if (getKeyState(KEY_Tilde, cabine_idx))
            rb[cabine_idx][RBP].set();
        else
            rb[cabine_idx][RBP].reset();

        // Включение/выключение ЭПK
        if (getKeyState(KEY_N, cabine_idx))
        {
            if (isShift())
                key_epk[cabine_idx].set();
            else
                key_epk[cabine_idx].reset();
        }

        // Прожектор "Тускло"
        if (getKeyState(KEY_G, cabine_idx))
        {
            if (isShift())
                spotlight_low_tumbler[cabine_idx].set();
            else
                spotlight_low_tumbler[cabine_idx].reset();
        }

        // Прожектор "Ярко"
        if (getKeyState(KEY_H, cabine_idx))
        {
            if (isShift())
                spotlight_high_tumbler[cabine_idx].set();
            else
                spotlight_high_tumbler[cabine_idx].reset();
        }

        // Включение/выключение ЭПТ
        if (getKeyState(KEY_V, cabine_idx))
        {
            if (isShift())
                epb_switch[cabine_idx].set();
            else
                epb_switch[cabine_idx].reset();
        }
    }
}
