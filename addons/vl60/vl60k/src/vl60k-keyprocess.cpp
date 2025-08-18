#include    "vl60k.h"

#include    "key-symbols.h"
#include    "timer.h"

#include "coupling-operating-rod.h"
#include "brake-crane.h"
#include "brake-lock.h"
#include "loco-crane.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose.h"
#include "sanding-system.h"
#include "train-horn.h"

#include "kme-60-044.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::keyProcess()
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


    // Управление оборудованием в кабинах
    for (auto cab_idx : {CAB1, CAB2})
    {
        // Управляем блокировкой тормозов
        brake_lock[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Управляем краном, учитывая возможное наличие внешнего пульта
        // TODO // перенести freejoy во вьювер, его команды передавать по сети,
        // TODO // и также указывая индекс кабины
        if (control_signals.analogSignal[CS_BRAKE_CRANE].is_active)
        {
            int brake_crane_pos = static_cast<int>(control_signals.analogSignal[CS_BRAKE_CRANE].cur_value);
            brake_crane[cab_idx]->setHandlePosition(brake_crane_pos);
        }
        else
        {
            brake_crane[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);
        }

        // Управляем краном, учитывая возможное наличие внешнего пульта
        // TODO // перенести freejoy во вьювер, его команды передавать по сети,
        // TODO // и также указывая индекс кабины
        if (control_signals.analogSignal[CS_LOCO_CRANE].is_active)
        {
            double pos = 0.0;

            if (static_cast<bool>(control_signals.analogSignal[CS_RELEASE_VALVE].cur_value))
            {
                loco_crane[cab_idx]->release(true);
                pos = -1.0;
            }
            else
            {
                loco_crane[cab_idx]->release(false);
                pos = control_signals.analogSignal[CS_LOCO_CRANE].cur_value;
            }

            loco_crane[cab_idx]->setHandlePosition(pos);
        }
        else
        {
            loco_crane[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);
        }

        // Тифон и свисток
        horn[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);
    }

    // Автозапуск
    if (autoStartTimer->isStarted())
    {
        controller[CAB1]->setControl();
        controller[CAB2]->setControl();
        return;
    }

    if (getKeyState(KEY_R, CAB1) && isAlt(CAB1) && initAutostartProgram(CAB1))
    {
        autoStartTimer->start();

        controller[CAB1]->setControl();
        controller[CAB2]->setControl();
        return;
    }

    if (getKeyState(KEY_R, CAB2) && isAlt(CAB2) && initAutostartProgram(CAB2))
    {
        autoStartTimer->start();

        controller[CAB1]->setControl();
        controller[CAB2]->setControl();
        return;
    }

    // Контроллер машиниста обрабатываем уже после проверки на невмешательство программы автозапуска
    if (controller[CAB1]->isReversHandle())
    {
        if (controller[CAB2]->isReversHandle())
        {
            // Не допускаем двух реверсивных рукояток
            controller[CAB2]->setMainHandlePos(POS_ZERO);
            controller[CAB2]->setReversHandlePos(REVERS_ZERO);
            controller[CAB2]->insertReversHandle(false);
        }
        controller[CAB2]->setControl();
    }
    else
    {
        controller[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);
    }

    if (controller[CAB2]->isReversHandle())
    {
        if (controller[CAB1]->isReversHandle())
        {
            // Не допускаем двух реверсивных рукояток
            controller[CAB1]->setMainHandlePos(POS_ZERO);
            controller[CAB1]->setReversHandlePos(REVERS_ZERO);
            controller[CAB1]->insertReversHandle(false);
        }
        controller[CAB1]->setControl();
    }
    else
    {
        controller[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
    }

    // Пульты в кабинах обрабатываем уже после проверки на невмешательство программы автозапуска
    for (auto cab_idx : {CAB1, CAB2})
    {
        // Дальний ряд тумблеров приборной панели машиниста
        spotlight_high_tumbler[cab_idx].step();
        spotlight_low_tumbler[cab_idx].step();
        radio_tumbler[cab_idx].step();
        cu_tumbler[cab_idx].step();
        pant2_tumbler[cab_idx].step();
        pant1_tumbler[cab_idx].step();
        pants_tumbler[cab_idx].step();
        gv_return_tumbler[cab_idx].step();
        gv_tumbler[cab_idx].step();

        // Ближний ряд тумблеров приборной панели машиниста
        autosand_tumbler[cab_idx].step();
        mv_tumblers[cab_idx][MV1].step();
        mv_tumblers[cab_idx][MV2].step();
        mv_tumblers[cab_idx][MV3].step();
        mv_tumblers[cab_idx][MV4].step();
        mv_tumblers[cab_idx][MV5].step();
        mv_tumblers[cab_idx][MV6].step();
        mk_tumbler[cab_idx].step();
        fr_tumbler[cab_idx].step();

        // Ряд тумблеров на приборной панели помощника машиниста
        P_tifon_tumbler[cab_idx].step();
        P_whistle_tumbler[cab_idx].step();
        P_cab_heat_tumbler[cab_idx].step();
        P_cab_light_low_tumbler[cab_idx].step();
        P_cab_light_high_tumbler[cab_idx].step();
        P_reserv1_tumbler[cab_idx].step();
        P_light_chassis_tumbler[cab_idx].step();
        P_light_devices_tumbler[cab_idx].step();
        P_bufferlight_L_tumbler[cab_idx].step();
        P_bufferlight_R_tumbler[cab_idx].step();
        P_reserv2_tumbler[cab_idx].step();
        P_ALSN_check_tumbler[cab_idx].step();
        P_buffercolor_L_toogle[cab_idx].step();
        P_buffercolor_R_toogle[cab_idx].step();

        // Нажатие РБС
        // Если активна РБС на внешнем пульте
        // TODO // перенести freejoy во вьювер, его команды передавать по сети,
        // TODO // и также указывая индекс кабины
        if (control_signals.analogSignal[CS_RBS].is_active)
        {
            // реагируем на состояние РБС на внешнем пульте
            if (static_cast<bool>(control_signals.analogSignal[CS_RBS].cur_value))
                rb[cab_idx][RBS].set();
            else
                rb[cab_idx][RBS].reset();
        }
        else // иначе
        {
            // обрабатываем клавиши
            rb[cab_idx][RBS].step();
        }
        rb[cab_idx][RB_1].step();
        rb[cab_idx][RBP].step();
        key_epk[cab_idx].step();
    }
}
