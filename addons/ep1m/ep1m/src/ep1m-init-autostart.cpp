#include    <ep1m.h>
#include    <km-35-01.h>
#include    <tumblers-panel.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1m::initAutostartProgram(int cab_autostart_request)
{
    if (autoStartTimer->isStarted())
    {
        return false;
    }

    autostart_shutdown = false;

    // проверяем индекс кабины
    if ((cab_autostart_request != CAB1) && (cab_autostart_request != CAB2))
    {
        return false;
    }

    // проверяем наличие ключа именно в рабочей кабине
    if (tumblers_panel[(cab_autostart_request == CAB1) ? CAB2 : CAB1]->isKey())
    {
        return false;
    }

    // проверяем наличие реверсивки именно в рабочей кабине
    if (km[(cab_autostart_request == CAB1) ? CAB2 : CAB1]->isReversHandle())
    {
        return false;
    }

    // проверяем блокировку 367 в рабочей кабине
    if (!brake_lock[cab_autostart_request]->isLockHandleAllowed())
    {
        return false;
    }

    // ключ ЭПК в рабочей кабине?
    if (!epk[cab_autostart_request]->isKeyAllowed())
    {
        return false;
    }

    autostart_cab = cab_autostart_request;
    tumblers_panel[autostart_cab]->insertKey(true);
    km[autostart_cab]->insertReversHandle(true);
    brake_lock[autostart_cab]->setStateOn(true);
    epk[autostart_cab]->insertKey(true);

    km[CAB1]->setControl();
    km[CAB2]->setControl();
    brake_lock[CAB1]->setControl();
    brake_lock[CAB2]->setControl();
    epk[CAB1]->setControl();
    epk[CAB2]->setControl();

    start_count = 0;
    autostart_triggers.clear();
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_LOCK_VVK));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MSUD));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_PANT2));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_AUX_MACHINES));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_COMPRESSOR));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN1));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN2));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN3));

    if (!epk[autostart_cab]->isKeyOn())
    {
        autostart_triggers.push_back(&tumblers[BUTTON_RBS][autostart_cab]);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1m::initShutdownProgram(int cab_shutdown_request)
{
    if (autoStartTimer->isStarted())
    {
        return false;
    }

    // проверяем индекс кабины
    if ((cab_shutdown_request != CAB1) && (cab_shutdown_request != CAB2))
    {
        return false;
    }

    // проверяем наличие ключа именно в рабочей кабине
    if (tumblers_panel[(cab_shutdown_request == CAB1) ? CAB2 : CAB1]->isKey())
    {
        return false;
    }

    // проверяем наличие реверсивки именно в рабочей кабине
    if (km[(cab_shutdown_request == CAB1) ? CAB2 : CAB1]->isReversHandle())
    {
        return false;
    }

    // проверяем блокировку 367 в рабочей кабине
    if (!brake_lock[cab_shutdown_request]->isLockHandleAllowed())
    {
        return false;
    }

    // ключ ЭПК в рабочей кабине?
    if (!epk[cab_shutdown_request]->isKeyAllowed())
    {
        return false;
    }

    // выключать нечего
    if (!tumblers_panel[cab_shutdown_request]->isKey() &&
        !km[cab_shutdown_request]->isReversHandle())
    {
        return false;
    }

    autostart_shutdown = true;
    autostart_cab = cab_shutdown_request;

    km[CAB1]->setControl();
    km[CAB2]->setControl();
    brake_lock[CAB1]->setControl();
    brake_lock[CAB2]->setControl();
    epk[CAB1]->setControl();
    epk[CAB2]->setControl();

    start_count = 0;
    autostart_triggers.clear();
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN3));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN2));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MOTOR_FAN1));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_COMPRESSOR));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_AUX_MACHINES));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_PANT2));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MSUD));
    autostart_triggers.push_back(tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_LOCK_VVK));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::slotAutostart()
{
    if (autostart_shutdown)
    {
        stepShutdownSequence();
        return;
    }

    if (start_count < autostart_triggers.size())
    {
        // Разблокируем панель тумблеров, если она не разблокирована
        if (!tumblers_panel[autostart_cab]->isKeyOn())
        {
            tumblers_panel[autostart_cab]->setKeyOn(true);
            return;
        }

        // Проверяем факт поднятия рогов, прежде чем включать ГВ
        if (autostart_triggers[start_count] == tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH) &&
            !pant[PANT1]->isUp() && !pant[PANT2]->isUp())
        {
            return;
        }

        // Проверяем, включен ли ГВ, чтобы отпустить возврат защиты
        if (main_switch->getState())
        {
            tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION)->reset();
        }
        else
        {
            if (autostart_triggers[start_count] == tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION))
            {
                tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION)->set();
                return;
            }
        }

        if ( (autostart_triggers[start_count] == &tumblers[BUTTON_RBS][autostart_cab]) &&
            !epk[autostart_cab]->isKeyOn())
        {
            epk[autostart_cab]->setKeyOn(true);
            return;
        }

        autostart_triggers[start_count++]->set();
    }
    else
    {
        autoStartTimer->stop();
        start_count = 0;

        km[autostart_cab]->setReversFwd();

        km[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
        km[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);
        brake_lock[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
        brake_lock[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);
        epk[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
        epk[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);

        if (auto_start_autopilot)
        {
            // Включаем модули автоведения, если мы - бот
            autopilot_switcher[CAB1].set();
            autopilot_switcher[CAB2].set();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepShutdownSequence()
{
    if (start_count < autostart_triggers.size())
    {
        // Последовательно отключаем тумблеры без контроля выполнения
        autostart_triggers[start_count++]->reset();
        return;
    }

    // Гасим оставшиеся тумблеры панели
    tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_PANT1)->reset();
    tumblers_panel[autostart_cab]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_EPT)->reset();

    // Блокируем панель тумблеров (ключ оставляем на месте)
    tumblers_panel[autostart_cab]->setKeyOn(false);

    // Возвращаем реверсор в ноль (рукоятку не извлекаем)
    km[autostart_cab]->setReversZero();

    // Выключаем автостоп (ключ оставляем на месте)
    epk[autostart_cab]->setKeyOn(false);

    // Возвращаем управление в кабины
    km[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
    km[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);
    brake_lock[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
    brake_lock[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);
    epk[CAB1]->setControl(&pressed_keys_by_cabine[CAB1]);
    epk[CAB2]->setControl(&pressed_keys_by_cabine[CAB2]);

    // Выключаем автоведение, если оно включено
    if (autopilot_switcher[CAB1].getState())
    {
        autopilot_switcher[CAB1].reset();
    }

    if (autopilot_switcher[CAB2].getState())
    {
        autopilot_switcher[CAB2].reset();
    }

    // Выключаем питание шкафа ШП-21
    tumbler_power_supply.reset();

    // Останавливаем таймер и сбрасываем состояние последовательности
    autoStartTimer->stop();
    start_count = 0;
    autostart_shutdown = false;
}
