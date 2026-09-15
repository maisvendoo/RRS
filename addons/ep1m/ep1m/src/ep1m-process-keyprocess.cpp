#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::keyProcess(const simulator_time_t& t, const double& dt)
{
    // Не допускаем двух рукояток в устройствах блокировки тормозов
    brake_lock[CAB2]->allowLockHandle(!(brake_lock[CAB1]->isLockHandle()));
    brake_lock[CAB1]->allowLockHandle(!(brake_lock[CAB2]->isLockHandle()));

    // Не допускаем двух ключей в электропневматических клапанах автостопа
    epk[CAB2]->allowKey(!(epk[CAB1]->isKey()));
    epk[CAB1]->allowKey(!(epk[CAB2]->isKey()));

    // Не допускаем двух ключей в панелях тумблеров
    tumblers_panel[CAB2]->allowKey(!(tumblers_panel[CAB1]->isKey()));
    tumblers_panel[CAB1]->allowKey(!(tumblers_panel[CAB2]->isKey()));

    // Не допускаем двух реверсивных рукояток в контроллерах машиниста
    km[CAB2]->allowReversHandle(!(km[CAB1]->isReversHandle()));
    km[CAB1]->allowReversHandle(!(km[CAB2]->isReversHandle()));

    tumbler_power_supply.step(t.simulation_seconds, dt);

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

    // Автовыключение
    if (getKeyState(KEY_T, CAB1) && isAlt(CAB1) && initShutdownProgram(CAB1))
    {
        autoStartTimer->start();
        return;
    }

    if (getKeyState(KEY_T, CAB2) && isAlt(CAB2) && initShutdownProgram(CAB2))
    {
        autoStartTimer->start();
        return;
    }

    // Управление оборудованием в кабинах
    for (auto cab_idx : {CAB1, CAB2})
    {
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

        // Тумблеры в кабинах
        tumblers_panel[cab_idx]->step(t.simulation_seconds, dt);

        for (size_t i = 0; i < TUMBLERS_COUNT; ++i)
        {
            if ((i == BUTTON_RBS) && control_signals.analogSignal[CS_RBS].is_active)
            {
                // реагируем на состояние РБС на внешнем пульте
                if (static_cast<bool>(control_signals.analogSignal[CS_RBS].cur_value))
                    tumblers[BUTTON_RBS][cab_idx].set();
                else
                    tumblers[BUTTON_RBS][cab_idx].reset();
            }
            else
            {
                tumblers[i][cab_idx].step(t.simulation_seconds, dt);
            }
        }

        for (size_t i = 0; i < SWITCHERS_COUNT; ++i)
        {
            switchers[i][cab_idx].step(t.simulation_seconds, dt);
        }

        brake_lock_door[cab_idx].step(t.simulation_seconds, dt);

        autopilot_switcher[cab_idx].step(t.simulation_seconds, dt);

        shunting_mode_switcher[cab_idx].step(t.simulation_seconds, dt);
    }    
}
