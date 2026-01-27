#include    "vl60pk.h"

#include    "key-symbols.h"
#include    "timer.h"

#include "automatic-train-stop.h"
#include "brake-crane.h"
#include "loco-crane.h"
#include "pneumo-brake-lock.h"
#include "sanding-system.h"
#include "train-horn.h"

#include "kme-60-044.h"
//#include "shield-223.h"
//#include "shield-225.h"
//#include "shield-229.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::keyProcess(const simulator_time_t& t, const double& dt)
{
    // Песочница
    sand_system->setControl(&pressed_keys);

    // Не допускаем двух рукояток в устройствах блокировки тормозов
    brake_lock[CAB2]->allowLockHandle(!(brake_lock[CAB1]->isLockHandle()));
    brake_lock[CAB1]->allowLockHandle(!(brake_lock[CAB2]->isLockHandle()));

    // Не допускаем двух ключей в электропневматических клапанах автостопа
    epk[CAB2]->allowKey(!(epk[CAB1]->isKey()));
    epk[CAB1]->allowKey(!(epk[CAB2]->isKey()));

    // Управление тормозным оборудованием в кабинах
    for (auto cabine_idx : {CAB1, CAB2})
    {
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

    // Автозапуск
    if (autoStartTimer->isStarted())
    {
        return;
    }

    if (getKeyState(KEY_R, CAB1) && isAlt(CAB1) && initAutostartProgram(CAB1))
    {
        autoStartTimer->start();
        return;
    }

    if (getKeyState(KEY_R, CAB2) && isAlt(CAB2) && initAutostartProgram(CAB2))
    {
        autoStartTimer->start();
        return;
    }

    // Контроллер машиниста обрабатываем уже после проверки на невмешательство программы автозапуска
    // Не допускаем двух реверсивных рукояток в контроллерах машиниста
    controller[CAB2]->allowReversHandle(!(controller[CAB1]->isReversHandle()));
    controller[CAB1]->allowReversHandle(!(controller[CAB2]->isReversHandle()));
/*
    // Не допускаем двух ключей в панелях тумблеров
    shield223[CAB2].allowKey(!(shield223[CAB1].isKey()));
    shield225[CAB2].allowKey(!(shield225[CAB1].isKey()));
    shield223[CAB1].allowKey(!(shield223[CAB2].isKey()));
    shield225[CAB1].allowKey(!(shield225[CAB2].isKey()));
*/
    // Пульты в кабинах обрабатываем уже после проверки на невмешательство программы автозапуска
    for (auto cab_idx : {CAB1, CAB2})
    {
        controller[cab_idx]->step(t.simulation_seconds, dt);
/*
        // Дальний ряд тумблеров приборной панели машиниста
        shield223[cab_idx].step(t.simulation_seconds, dt);
        // Ближний ряд тумблеров приборной панели машиниста
        shield225[cab_idx].step(t.simulation_seconds, dt);
        // Ряд тумблеров на приборной панели помощника машиниста
        shield229[cab_idx].step(t.simulation_seconds, dt);
*/
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
        epb_switch[cab_idx].step();

        autopilot_switcher[cab_idx].step();

        tumbler_shunting_mode[cab_idx].step();
    }    
}
