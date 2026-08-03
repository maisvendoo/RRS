#include    "vl60pk.h"

#include    "key-symbols.h"
#include    "coupling-operating-rod.h"
#include    "automatic-train-stop.h"
#include    "pneumo-brake-lock.h"
#include    "pneumo-anglecock.h"
#include    "pneumo-hose-epb.h"
#include    "kme-60-044.h"
//#include    "shield-223.h"
//#include    "shield-225.h"
//#include    "shield-229.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    // Расцепные рычаги
    oper_rod_fwd->setKeySymbol(KEY_X);
    oper_rod_fwd->setControl(&pressed_keys);

    oper_rod_bwd->setKeySymbol(KEY_C);
    oper_rod_bwd->setControl(&pressed_keys);

    // Концевые краны магистрали тормозных цилиндров
    anglecock_bc_fwd->setKeySymbolOpen(KEY_F10);
    anglecock_bc_fwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bc_fwd->setKeySymbolClose(KEY_F10);
    anglecock_bc_fwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bc_fwd->setControl(&pressed_keys);

    anglecock_bc_bwd->setKeySymbolOpen(KEY_F11);
    anglecock_bc_bwd->setKeyModifierOpen(MODIFIER_OnlyShift);
    anglecock_bc_bwd->setKeySymbolClose(KEY_F11);
    anglecock_bc_bwd->setKeyModifierClose(MODIFIER_OnlyControl);
    anglecock_bc_bwd->setControl(&pressed_keys);

    // Рукава магистрали тормозных цилиндров
    hose_bc_fwd->setKeySymbolConnect(KEY_F9);
    hose_bc_fwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bc_fwd->setKeySymbolDisconnect(KEY_F9);
    hose_bc_fwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bc_fwd->setControl(&pressed_keys);

    hose_bc_bwd->setKeySymbolConnect(KEY_F12);
    hose_bc_bwd->setKeyModifierConnect(MODIFIER_OnlyShift);
    hose_bc_bwd->setKeySymbolDisconnect(KEY_F12);
    hose_bc_bwd->setKeyModifierDisconnect(MODIFIER_OnlyControl);
    hose_bc_bwd->setControl(&pressed_keys);

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


    for (auto cab_idx : {CAB1, CAB2})
    {
        // Контроллер машиниста
        controller[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Устройство блокировки тормозов усл.№ 367
        brake_lock[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);

        // Электропневматический клапан автостопа
        epk[cab_idx]->setControl(&pressed_keys_by_cabine[cab_idx]);
/*
        // Дальний ряд тумблеров приборной панели машиниста
        shield223[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Ближний ряд тумблеров приборной панели машиниста
        shield225[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        // Ряд тумблеров на приборной панели помощника машиниста
        shield229[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
*/
        // Дальний ряд тумблеров приборной панели машиниста
        // Триггер тумблера "Прожектор яркий"
        spotlight_high_tumbler[cab_idx].setKeySymbolOn(KEY_H);
        spotlight_high_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
        spotlight_high_tumbler[cab_idx].setKeySymbolOff(KEY_H);
        spotlight_high_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
        spotlight_high_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Прожектор тусклый"
        spotlight_low_tumbler[cab_idx].setKeySymbolOn(KEY_H);
        spotlight_low_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        spotlight_low_tumbler[cab_idx].setKeySymbolOff(KEY_H);
        spotlight_low_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        spotlight_low_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Радиостанция"
        radio_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Цепи управления"
        cu_tumbler[cab_idx].setKeySymbolOn(KEY_Y);
        cu_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        cu_tumbler[cab_idx].setKeySymbolOff(KEY_Y);
        cu_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        cu_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Токоприемник задний"
        pant2_tumbler[cab_idx].setKeySymbolOn(KEY_O);
        pant2_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        pant2_tumbler[cab_idx].setKeySymbolOff(KEY_O);
        pant2_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        pant2_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Токоприемник передний"
        pant1_tumbler[cab_idx].setKeySymbolOn(KEY_I);
        pant1_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        pant1_tumbler[cab_idx].setKeySymbolOff(KEY_I);
        pant1_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        pant1_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Токоприемники"
        /*pants_tumbler[cab_idx].setKeySymbolOn(KEY_U);
        pants_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        pants_tumbler[cab_idx].setKeySymbolOff(KEY_U);
        pants_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        pants_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);*/

        // Триггер тумблера "ГВ вкл. Возврат защиты"
        gv_return_tumbler[cab_idx].setKeySymbolOn(KEY_P);
        gv_return_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
        gv_return_tumbler[cab_idx].setKeySymbolOff(KEY_Undefined);
        gv_return_tumbler[cab_idx].setKeyModifierOff(KEY_Undefined);
        gv_return_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "ГВ вкл/выкл"
        gv_tumbler[cab_idx].setKeySymbolOn(KEY_P);
        gv_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        gv_tumbler[cab_idx].setKeySymbolOff(KEY_P);
        gv_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        gv_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Ближний ряд тумблеров приборной панели машиниста
        // Триггер тумблера "Автоматическая подача песка"
        autosand_tumbler[cab_idx].setInitState(false);

        // Триггеры тумблеров "Вентилятор 1-6"
        mv_tumblers[cab_idx][MV1].setKeySymbolOn(KEY_1);
        mv_tumblers[cab_idx][MV1].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV1].setKeySymbolOff(KEY_1);
        mv_tumblers[cab_idx][MV1].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV1].setControl(&pressed_keys_by_cabine[cab_idx]);

        mv_tumblers[cab_idx][MV2].setKeySymbolOn(KEY_2);
        mv_tumblers[cab_idx][MV2].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV2].setKeySymbolOff(KEY_2);
        mv_tumblers[cab_idx][MV2].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV2].setControl(&pressed_keys_by_cabine[cab_idx]);

        mv_tumblers[cab_idx][MV3].setKeySymbolOn(KEY_3);
        mv_tumblers[cab_idx][MV3].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV3].setKeySymbolOff(KEY_3);
        mv_tumblers[cab_idx][MV3].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV3].setControl(&pressed_keys_by_cabine[cab_idx]);

        mv_tumblers[cab_idx][MV4].setKeySymbolOn(KEY_4);
        mv_tumblers[cab_idx][MV4].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV4].setKeySymbolOff(KEY_4);
        mv_tumblers[cab_idx][MV4].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV4].setControl(&pressed_keys_by_cabine[cab_idx]);

        mv_tumblers[cab_idx][MV5].setKeySymbolOn(KEY_5);
        mv_tumblers[cab_idx][MV5].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV5].setKeySymbolOff(KEY_5);
        mv_tumblers[cab_idx][MV5].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV5].setControl(&pressed_keys_by_cabine[cab_idx]);

        mv_tumblers[cab_idx][MV6].setKeySymbolOn(KEY_6);
        mv_tumblers[cab_idx][MV6].setKeyModifierOn(MODIFIER_OnlyShift);
        mv_tumblers[cab_idx][MV6].setKeySymbolOff(KEY_6);
        mv_tumblers[cab_idx][MV6].setKeyModifierOff(MODIFIER_OnlyControl);
        mv_tumblers[cab_idx][MV6].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Компрессор"
        mk_tumbler[cab_idx].setKeySymbolOn(KEY_7);
        mk_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        mk_tumbler[cab_idx].setKeySymbolOff(KEY_7);
        mk_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        mk_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Фазорасщепитель"
        fr_tumbler[cab_idx].setKeySymbolOn(KEY_T);
        fr_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        fr_tumbler[cab_idx].setKeySymbolOff(KEY_T);
        fr_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        fr_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Ряд тумблеров на приборной панели помощника машиниста
        // Триггер тумблера "Тифон"
        P_tifon_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Свисток"
        P_whistle_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Обогрев кабины"
        P_cab_heat_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Тусклое освещение кабины"
        P_cab_light_low_tumbler[cab_idx].setKeySymbolOn(KEY_K);
        P_cab_light_low_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        P_cab_light_low_tumbler[cab_idx].setKeySymbolOff(KEY_K);
        P_cab_light_low_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        P_cab_light_low_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Яркое освещение кабины"
        P_cab_light_high_tumbler[cab_idx].setKeySymbolOn(KEY_K);
        P_cab_light_high_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
        P_cab_light_high_tumbler[cab_idx].setKeySymbolOff(KEY_K);
        P_cab_light_high_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
        P_cab_light_high_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера в резерве
        P_reserv1_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Освещение ходовой"
        P_light_chassis_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Освещение приборов"
        P_light_devices_tumbler[cab_idx].setKeySymbolOn(KEY_L);
        P_light_devices_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        P_light_devices_tumbler[cab_idx].setKeySymbolOff(KEY_L);
        P_light_devices_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        P_light_devices_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Фонарь левый буферный"
        P_bufferlight_L_tumbler[cab_idx].setKeySymbolOn(KEY_G);
        P_bufferlight_L_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        P_bufferlight_L_tumbler[cab_idx].setKeySymbolOff(KEY_G);
        P_bufferlight_L_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        P_bufferlight_L_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера "Фонарь правый буферный"
        P_bufferlight_R_tumbler[cab_idx].setKeySymbolOn(KEY_J);
        P_bufferlight_R_tumbler[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        P_bufferlight_R_tumbler[cab_idx].setKeySymbolOff(KEY_J);
        P_bufferlight_R_tumbler[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        P_bufferlight_R_tumbler[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Триггер тумблера в резерве
        P_reserv2_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Проверка АЛСН"
        P_ALSN_check_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Левый буферный белый/красный"
        P_buffercolor_L_toogle[cab_idx].setKeySymbolOn(KEY_G);
        P_buffercolor_L_toogle[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
        P_buffercolor_L_toogle[cab_idx].setKeySymbolOff(KEY_G);
        P_buffercolor_L_toogle[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
        P_buffercolor_L_toogle[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        P_buffercolor_L_toogle[cab_idx].setInitState(true);

        // Триггер тумблера "Правый буферный белый/красный"
        P_buffercolor_R_toogle[cab_idx].setKeySymbolOn(KEY_J);
        P_buffercolor_R_toogle[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
        P_buffercolor_R_toogle[cab_idx].setKeySymbolOff(KEY_J);
        P_buffercolor_R_toogle[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
        P_buffercolor_R_toogle[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
        P_buffercolor_R_toogle[cab_idx].setInitState(true);


        // Триггеры рукояток бдительности
        rb[cab_idx][RBS].setKeySymbolOn(KEY_M);
        rb[cab_idx][RBS].setKeyModifierOn(ANY_MODIFIERS);
        rb[cab_idx][RBS].setKeySymbolOff(KEY_Undefined);
        rb[cab_idx][RBS].setKeyModifierOff(KEY_Undefined);
        rb[cab_idx][RBS].setControl(&pressed_keys_by_cabine[cab_idx]);

        rb[cab_idx][RB_1].setKeySymbolOn(KEY_Z);
        rb[cab_idx][RB_1].setKeyModifierOn(ANY_MODIFIERS);
        rb[cab_idx][RB_1].setKeySymbolOff(KEY_Undefined);
        rb[cab_idx][RB_1].setKeyModifierOff(KEY_Undefined);
        rb[cab_idx][RB_1].setControl(&pressed_keys_by_cabine[cab_idx]);

        rb[cab_idx][RBP].setKeySymbolOn(KEY_Tilde);
        rb[cab_idx][RBP].setKeyModifierOn(ANY_MODIFIERS);
        rb[cab_idx][RBP].setKeySymbolOff(KEY_Undefined);
        rb[cab_idx][RBP].setKeyModifierOff(KEY_Undefined);
        rb[cab_idx][RBP].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер ЭПТ
        epb_switch[cab_idx].setKeySymbolOn(KEY_V);
        epb_switch[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        epb_switch[cab_idx].setKeySymbolOff(KEY_V);
        epb_switch[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        epb_switch[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);

        // Тумблер "Маневровый/Поездной"
        tumbler_shunting_mode[cab_idx].setKeySymbolOn(KEY_Tilde);
        tumbler_shunting_mode[cab_idx].setKeyModifierOn(MODIFIER_OnlyShift);
        tumbler_shunting_mode[cab_idx].setKeySymbolOff(KEY_Tilde);
        tumbler_shunting_mode[cab_idx].setKeyModifierOff(MODIFIER_OnlyControl);
        tumbler_shunting_mode[cab_idx].setControl(&pressed_keys_by_cabine[cab_idx]);
    }
}
