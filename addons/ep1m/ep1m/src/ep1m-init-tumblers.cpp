#include    "ep1m.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void EP1m::initControl(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Песочница
    sand_system->setControl(&pressed_keys);

    // Тифон и свисток
    //horn->setControl(&pressed_keys);

    // Расцепные рычаги
    oper_rod_fwd->setKeySymbol(KEY_X);
    oper_rod_fwd->setControl(&pressed_keys);

    oper_rod_bwd->setKeySymbol(KEY_C);
    oper_rod_bwd->setControl(&pressed_keys);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd->setKeySymbolOpen(KEY_F6);
    anglecock_fl_fwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_fl_fwd->setKeySymbolClose(KEY_F6);
    anglecock_fl_fwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_fl_fwd->setControl(&pressed_keys);

    anglecock_fl_bwd->setKeySymbolOpen(KEY_F7);
    anglecock_fl_bwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_fl_bwd->setKeySymbolClose(KEY_F7);
    anglecock_fl_bwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_fl_bwd->setControl(&pressed_keys);

    // Рукава питательной магистрали
    hose_fl_fwd->setKeySymbolConnect(KEY_F5);
    hose_fl_fwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_fl_fwd->setKeySymbolDisconnect(KEY_F5);
    hose_fl_fwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_fl_fwd->setControl(&pressed_keys);

    hose_fl_bwd->setKeySymbolConnect(KEY_F8);
    hose_fl_bwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_fl_bwd->setKeySymbolDisconnect(KEY_F8);
    hose_fl_bwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_fl_bwd->setControl(&pressed_keys);

    // Концевые краны тормозной магистрали
    anglecock_bp_fwd->setKeySymbolOpen(KEY_F2);
    anglecock_bp_fwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bp_fwd->setKeySymbolClose(KEY_F2);
    anglecock_bp_fwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bp_fwd->setControl(&pressed_keys);

    anglecock_bp_bwd->setKeySymbolOpen(KEY_F3);
    anglecock_bp_bwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bp_bwd->setKeySymbolClose(KEY_F3);
    anglecock_bp_bwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bp_bwd->setControl(&pressed_keys);

    // Рукава тормозной магистрали
    hose_bp_fwd->setKeySymbolConnect(KEY_F1);
    hose_bp_fwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bp_fwd->setKeySymbolDisconnect(KEY_F1);
    hose_bp_fwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bp_fwd->setControl(&pressed_keys);

    hose_bp_bwd->setKeySymbolConnect(KEY_F4);
    hose_bp_bwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bp_bwd->setKeySymbolDisconnect(KEY_F4);
    hose_bp_bwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bp_bwd->setControl(&pressed_keys);


    // Выключатель шкафа питания ШП-21
    tumbler_power_supply.setKeySymbolOn(KEY_U);
    tumbler_power_supply.setKeyModifierOn(MODIFIER_OnlyShift);
    tumbler_power_supply.setKeySymbolOff(KEY_U);
    tumbler_power_supply.setKeyModifierOff(MODIFIER_OnlyControl);
    tumbler_power_supply.setControl(&pressed_keys);

    // Управление ослаблением поля через МСУД
    msud->setControl(&pressed_keys);


    for (auto cab_idx : {CAB1, CAB2})
    {
        // Дверца тумбы с устройством блокировки тормозов
        brake_lock_door[cab_idx].setKeySymbolOn(KEY_Backslash);
        brake_lock_door[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        brake_lock_door[cab_idx].setKeySymbolOff(KEY_Backslash);
        brake_lock_door[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        brake_lock_door[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Устройство блокировки тормозов усл.№ 367
        brake_lock[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Электропневматический клапан автостопа
        epk[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Панель тумблеров
        tumblers_panel[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Контроллер
        km[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Панель на столешнице справа
        // Тумблер "Автоматическая подсыпка песка"
        // Не реализовано в схеме
        tumblers[TUMBLER_AUTOSAND][cab_idx].setInitState(false);

        // Тумблер "Подсыпка песка при экстренном торможении"
        // Не реализовано в схеме
        tumblers[TUMBLER_SAND_ON_EMERGENCY][cab_idx].setInitState(false);

        // Тумблер "Выбор комплекта: МПК-1 или МПК-2"
            // tumblers[TUMBLER_MPK][cab_idx].setKeySymbolOn(KEY_6);
    // tumblers[TUMBLER_MPK][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_MPK][cab_idx].setKeySymbolOff(KEY_6);
    // tumblers[TUMBLER_MPK][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_MPK][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Кнопка "Возврат реле (защиты вспом. машин)"
        // Не реализовано в схеме
        tumblers[BUTTON_RETURN_AUX_PROTECTION][cab_idx].setInitState(false);

        // Тумблер "Выбор режима управления"
            // tumblers[TUMBLER_AUTO_MODE][cab_idx].setKeySymbolOn(KEY_F);
    // tumblers[TUMBLER_AUTO_MODE][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_AUTO_MODE][cab_idx].setKeySymbolOff(KEY_F);
    // tumblers[TUMBLER_AUTO_MODE][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_AUTO_MODE][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Автоматическое регулирование по умолчанию
        tumblers[TUMBLER_AUTO_MODE][cab_idx].setInitState(true);

        // Панель на средней тумбе
        // Цвет левого буферного огня
            // tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setKeySymbolOn(KEY_G);
    // tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setKeySymbolOff(KEY_G);
    // tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Белый по умолчанию
        tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].setInitState(true);

        // Включение левого буферного огня
            // tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].setKeySymbolOn(KEY_G);
    // tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].setKeySymbolOff(KEY_G);
    // tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Включение правого буферного огня
            // tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].setKeySymbolOn(KEY_J);
    // tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].setKeySymbolOff(KEY_J);
    // tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Цвет правого буферного огня
            // tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setKeySymbolOn(KEY_J);
    // tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setKeySymbolOff(KEY_J);
    // tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Белый по умолчанию
        tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].setInitState(true);

        // Панель слева
        // Тумблер "Освещение тележек"
        // Не реализовано
        tumblers[TUMBLER_SHASSIS_LIGHT][cab_idx].setInitState(false);

        // Тумблер "Прожектор тусклый"
            // tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].setKeySymbolOn(KEY_H);
    // tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].setKeySymbolOff(KEY_H);
    // tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Прожектор яркий"
            // tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].setKeySymbolOn(KEY_H);
    // tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].setKeySymbolOff(KEY_H);
    // tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Освещение кабины тускло"
            // tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].setKeySymbolOn(KEY_K);
    // tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].setKeySymbolOff(KEY_K);
    // tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Освещение кабины ярко"
            // tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].setKeySymbolOn(KEY_K);
    // tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].setKeySymbolOff(KEY_K);
    // tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Освещение кабины зелёный свет"
            // tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].setKeySymbolOn(KEY_Backslash);
    // tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].setKeySymbolOff(KEY_Backslash);
    // tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Освещение приборов и пульта"
            // tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].setKeySymbolOn(KEY_L);
    // tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].setKeySymbolOff(KEY_L);
    // tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
    // tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Регулятор "Яркость освещения пульта"
        // Не реализовано
        switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].setNumPositions(3);
        // Автовозврат в среднее положение
        switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].setSpringFirst();
        switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].setSpringLast();
        switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].setInitPosition(1);

        // Регулятор "Яркость освещения приборов"
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setNumPositions(3);
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setKeySymbolIncrease(KEY_L);
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setKeyModifierIncrease(MODIFIER_OnlyShift);
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setKeySymbolDecrease(KEY_L);
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setKeyModifierDecrease(MODIFIER_OnlyControl);
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Автовозврат в среднее положение
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setSpringFirst();
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setSpringLast();
        switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].setInitPosition(1);

        // Тумблер "Напряжение батареи или напряжение СПН ЭПТ"
            // tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setKeySymbolOn(KEY_5);
    // tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setKeySymbolOff(KEY_5);
    // tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].setInitState(true);

        // Панель на столешнице слева
        // Кнопка "Отпуск тормозов"
        tumblers[BUTTON_RELEASE_BRAKES][cab_idx].setKeySymbolOn(KEY_Slash);
        tumblers[BUTTON_RELEASE_BRAKES][cab_idx].setKeyModifierOn(ANY_MODIFIERS);
        tumblers[BUTTON_RELEASE_BRAKES][cab_idx].setKeySymbolOff(KEY_Undefined);
        tumblers[BUTTON_RELEASE_BRAKES][cab_idx].setKeyModifierOff(KEY_Undefined);
        tumblers[BUTTON_RELEASE_BRAKES][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Кнопка "Компрессор"
        tumblers[BUTTON_COMPRESSOR][cab_idx].setKeySymbolOn(KEY_R);
        tumblers[BUTTON_COMPRESSOR][cab_idx].setKeyModifierOn(KEY_Undefined);
        tumblers[BUTTON_COMPRESSOR][cab_idx].setKeySymbolOff(KEY_Undefined);
        tumblers[BUTTON_COMPRESSOR][cab_idx].setKeyModifierOff(KEY_Undefined);
        tumblers[BUTTON_COMPRESSOR][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Сигнализация"
            // tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setKeySymbolOn(KEY_8);
    // tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setKeySymbolOff(KEY_8);
    // tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Включен по умолчанию
        tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].setInitState(true);

        // Тумблер "ПЧФ (преобразователь частоты фаз)"
            // tumblers[TUMBLER_PCHF][cab_idx].setKeySymbolOn(KEY_7);
    // tumblers[TUMBLER_PCHF][cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
    // tumblers[TUMBLER_PCHF][cab_idx].setKeySymbolOff(KEY_7);
    // tumblers[TUMBLER_PCHF][cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
    // tumblers[TUMBLER_PCHF][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Включен по умолчанию
        tumblers[TUMBLER_PCHF][cab_idx].setInitState(true);

        // Тумблер "УСАВП"
        // Не реализовано
        tumblers[TUMBLER_USAVP][cab_idx].setInitState(false);

        // Рукоятки бдительности
        tumblers[BUTTON_RB][cab_idx].setKeySymbolOn(KEY_Z);
        tumblers[BUTTON_RB][cab_idx].setKeyModifierOn(ANY_MODIFIERS);
        tumblers[BUTTON_RB][cab_idx].setKeySymbolOff(KEY_Undefined);
        tumblers[BUTTON_RB][cab_idx].setKeyModifierOff(KEY_Undefined);
        tumblers[BUTTON_RB][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        tumblers[BUTTON_RBS][cab_idx].setKeySymbolOn(KEY_M);
        tumblers[BUTTON_RBS][cab_idx].setKeyModifierOn(ANY_MODIFIERS);
        tumblers[BUTTON_RBS][cab_idx].setKeySymbolOff(KEY_Undefined);
        tumblers[BUTTON_RBS][cab_idx].setKeyModifierOff(KEY_Undefined);
        tumblers[BUTTON_RBS][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        //tumblers[BUTTON_RBP][cab_idx].setKeySymbolOn(KEY_Tilde);
        //tumblers[BUTTON_RBP][cab_idx].setKeyModifierOn(ANY_MODIFIERS);
        //tumblers[BUTTON_RBP][cab_idx].setKeySymbolOff(KEY_Undefined);
        //tumblers[BUTTON_RBP][cab_idx].setKeyModifierOff(KEY_Undefined);
        //tumblers[BUTTON_RBP][cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Пульт помощника
        // Кнопка "Экстренное торможение"
        // Реализована только проверка в схеме
        tumblers[BUTTON_EMERGENCY_BRAKE][cab_idx].setInitState(false);

        // Кнопка "Аварийное отключение главного выключателя"
        // Реализована только проверка в схеме
        tumblers[BUTTON_MAIN_SWITCH_OFF][cab_idx].setInitState(false);

        // Кнопка "Тифон"
        // У помощника не реализовано
        tumblers[BUTTON_P_TYPHON][cab_idx].setInitState(false);

        // Кнопка "Свисток"
        // У помощника не реализовано
        tumblers[BUTTON_P_WHISTLE][cab_idx].setInitState(false);

        shunting_mode_switcher[cab_idx].setKeySymbolOn(KEY_Tilde);
        shunting_mode_switcher[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        shunting_mode_switcher[cab_idx].setKeySymbolOff(KEY_Tilde);
        shunting_mode_switcher[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        shunting_mode_switcher[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тифон и свисток
        horn[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);
    }
}
