#include    <vl60k.h>
#include    <vl60-controls.h>
#include    <kme-60-044.h>
#include    <automatic-train-stop.h>
#include    <pneumo-brake-lock.h>
#include    <brake-crane.h>
#include    <loco-crane.h>
#include    <sanding-system.h>
#include    <train-horn.h>
#include    <pneumo-anglecock.h>
#include    <pneumo-hose.h>
#include    <coupling-operating-rod.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::stepControls(const double &t, const double &dt)
{
    // Не допускаем двух реверсивных рукояток в контроллерах машиниста
    controller[CAB2]->allowReversHandle(!(controller[CAB1]->isReversHandle()));
    controller[CAB1]->allowReversHandle(!(controller[CAB2]->isReversHandle()));

    // Не допускаем двух рукояток в устройствах блокировки тормозов
    brake_lock[CAB2]->allowLockHandle(!(brake_lock[CAB1]->isLockHandle()));
    brake_lock[CAB1]->allowLockHandle(!(brake_lock[CAB2]->isLockHandle()));

    // Не допускаем двух ключей в электропневматических клапанах автостопа
    epk[CAB2]->allowKey(!(epk[CAB1]->isKey()));
    epk[CAB1]->allowKey(!(epk[CAB2]->isKey()));

    for (auto cab_idx : {CAB1, CAB2})
    {
        // Шаг контроллера
        controller[cab_idx]->step(t, dt);

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

        autopilot_switcher[cab_idx].step();
        tumbler_shunting_mode[cab_idx].step();

        rb[cab_idx][RB_1].step();
        rb[cab_idx][RBS].step();
        rb[cab_idx][RBP].step();

        // Отсекаем управление при автозапуске или автоостанове
        if (autoStartTimer->isStarted() || autostart_mode != AUTOSTART_IDLE)
        {
            continue;
        }

        // Автозапуск по нажатию комбинации клавиш в текущей кабине
        if (vc_inputs.getBool(cab_idx, CTRL_AUTOSTART_PROGRAM))
        {
            if (initAutostartProgram(cab_idx))
            {
                autoStartTimer->start();
            }

            initClientInputSignal(cab_idx, CTRL_AUTOSTART_PROGRAM, 0.0f);

            return;
        }

        // Автоостанов по нажатию комбинации клавиш в текущей кабине
        if (vc_inputs.getBool(cab_idx, CTRL_AUTOSTOP_PROGRAM))
        {
            if (initAutostopProgram(cab_idx))
            {
                autoStartTimer->start();
            }

            initClientInputSignal(cab_idx, CTRL_AUTOSTOP_PROGRAM, 0.0f);

            return;
        }

        bool is_pnt = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_PNT);
        is_pnt ? pants_tumbler[cab_idx].set() : pants_tumbler[cab_idx].reset();

        bool is_pnt1 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_PNT1);
        is_pnt1 ? pant1_tumbler[cab_idx].set() : pant1_tumbler[cab_idx].reset();

        bool is_pnt2 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_PNT2);
        is_pnt2 ? pant2_tumbler[cab_idx].set() : pant2_tumbler[cab_idx].reset();

        bool is_main_switch_on = vc_inputs.getBool(cab_idx, CTRL_MAIN_SWITCH_ON);
        is_main_switch_on ? gv_tumbler[cab_idx].set() : gv_tumbler[cab_idx].reset();

        bool is_main_switch_return = vc_inputs.getBool(cab_idx, CTRL_RETURN_PROTECTION);
        is_main_switch_return ? gv_return_tumbler[cab_idx].set() : gv_return_tumbler[cab_idx].reset();

        bool is_fr = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_FR);
        is_fr ? fr_tumbler[cab_idx].set() : fr_tumbler[cab_idx].reset();

        bool is_mk = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MK);
        is_mk ? mk_tumbler[cab_idx].set() : mk_tumbler[cab_idx].reset();

        bool is_mv1 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV1);
        is_mv1 ? mv_tumblers[cab_idx][MV1].set() : mv_tumblers[cab_idx][MV1].reset();

        bool is_mv2 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV2);
        is_mv2 ? mv_tumblers[cab_idx][MV2].set() : mv_tumblers[cab_idx][MV2].reset();

        bool is_mv3 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV3);
        is_mv3 ? mv_tumblers[cab_idx][MV3].set() : mv_tumblers[cab_idx][MV3].reset();

        bool is_mv4 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV4);
        is_mv4 ? mv_tumblers[cab_idx][MV4].set() : mv_tumblers[cab_idx][MV4].reset();

        bool is_mv5 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV5);
        is_mv5 ? mv_tumblers[cab_idx][MV5].set() : mv_tumblers[cab_idx][MV5].reset();

        bool is_mv6 = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_MV6);
        is_mv6 ? mv_tumblers[cab_idx][MV6].set() : mv_tumblers[cab_idx][MV6].reset();

        bool is_cu = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_CU);
        is_cu ? cu_tumbler[cab_idx].set() : cu_tumbler[cab_idx].reset();

        bool is_spot_high = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_SPOT_HIGH);
        is_spot_high ? spotlight_high_tumbler[cab_idx].set() : spotlight_high_tumbler[cab_idx].reset();

        bool is_spot_low = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_SPOT_LOW);
        is_spot_low ? spotlight_low_tumbler[cab_idx].set() : spotlight_low_tumbler[cab_idx].reset();

        bool is_cab_light_low = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_CAB_LIGHT_LOW);
        is_cab_light_low ? P_cab_light_low_tumbler[cab_idx].set() : P_cab_light_low_tumbler[cab_idx].reset();

        bool is_cab_light_high = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_CAB_LIGHT_HIGH);
        is_cab_light_high ? P_cab_light_high_tumbler[cab_idx].set() : P_cab_light_high_tumbler[cab_idx].reset();

        bool is_light_devices = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_LIGHT_DEVICES);
        is_light_devices ? P_light_devices_tumbler[cab_idx].set() : P_light_devices_tumbler[cab_idx].reset();

        bool is_buf_light_l = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_BUF_LIGHT_L);
        is_buf_light_l ? P_bufferlight_L_tumbler[cab_idx].set() : P_bufferlight_L_tumbler[cab_idx].reset();

        bool is_buf_light_r = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_BUF_LIGHT_R);
        is_buf_light_r ? P_bufferlight_R_tumbler[cab_idx].set() : P_bufferlight_R_tumbler[cab_idx].reset();

        bool is_buf_color_l = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_BUF_COLOR_L);
        is_buf_color_l ? P_buffercolor_L_toogle[cab_idx].set() : P_buffercolor_L_toogle[cab_idx].reset();

        bool is_buf_color_r = vc_inputs.getBool(cab_idx, CTRL_TUMBLER_BUF_COLOR_R);
        is_buf_color_r ? P_buffercolor_R_toogle[cab_idx].set() : P_buffercolor_R_toogle[cab_idx].reset();

        // Радиостанция
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_RADIO) ? radio_tumbler[cab_idx].set() : radio_tumbler[cab_idx].reset();

        // Автоматическая подача песка
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_AUTOSAND) ? autosand_tumbler[cab_idx].set() : autosand_tumbler[cab_idx].reset();

        // Тифон и свисток (кнопки с самовозвратом)
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_TIFON) ? P_tifon_tumbler[cab_idx].set() : P_tifon_tumbler[cab_idx].reset();
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_WHISTLE) ? P_whistle_tumbler[cab_idx].set() : P_whistle_tumbler[cab_idx].reset();

        // Обогрев кабины
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_CAB_HEAT) ? P_cab_heat_tumbler[cab_idx].set() : P_cab_heat_tumbler[cab_idx].reset();

        // Освещение ходовой
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_SHASSIS_LIGHT) ? P_light_chassis_tumbler[cab_idx].set() : P_light_chassis_tumbler[cab_idx].reset();

        // Проверка АЛСН
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_ALSN_CHECK) ? P_ALSN_check_tumbler[cab_idx].set() : P_ALSN_check_tumbler[cab_idx].reset();

        // Резервные
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_RESERVE1) ? P_reserv1_tumbler[cab_idx].set() : P_reserv1_tumbler[cab_idx].reset();
        vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_RESERVE2) ? P_reserv2_tumbler[cab_idx].set() : P_reserv2_tumbler[cab_idx].reset();

        // Управление контроллером машиниста
        controller[cab_idx]->insertReversHandle(vc_inputs.getBool(cab_idx, CTRL_REVERS_INSERTION));

        if (!autopilot_switcher[cab_idx].getState())
        {
            controller[cab_idx]->setReversHandlePos(vc_inputs.getValue(cab_idx, CTRL_REVERS_POSITION));
            controller[cab_idx]->setMainHandlePos(vc_inputs.getValue(cab_idx, CTRL_KM_MAIN_POSITION));
            brake_crane[cab_idx]->setHandlePosition(vc_inputs.getValue(cab_idx, CTRL_KRM_395));
            loco_crane[cab_idx]->setHandlePosition(vc_inputs.getValue(cab_idx, CTRL_KVT_254));
        }
        else
        {
            initClientInputSignal(cab_idx, CTRL_REVERS_POSITION, controller[cab_idx]->getReversHandlePos());
            initClientInputSignal(cab_idx, CTRL_KM_MAIN_POSITION, controller[cab_idx]->getMainPos());
            initClientInputSignal(cab_idx, CTRL_KRM_395, brake_crane[cab_idx]->getHandlePosition());
            initClientInputSignal(cab_idx, CTRL_KVT_254, loco_crane[cab_idx]->getHandlePosition());
        }

        // Управление ЭПК
        bool is_epk_insert = vc_inputs.getBool(cab_idx, CTRL_EPK_INSERTION);
        epk[cab_idx]->insertKey(is_epk_insert);

        bool is_key_epk_ON = vc_inputs.getBool(cab_idx, CTRL_KEY_EPK);
        epk[cab_idx]->setKeyOn(is_key_epk_ON);

        // Управление блокировкой 367
        bool is_lock367_insert = vc_inputs.getBool(cab_idx, CTRL_LOCK_367_INSERTION);
        brake_lock[cab_idx]->setStateOn(is_lock367_insert);
        brake_lock[cab_idx]->insertLockHandle(is_lock367_insert);
        brake_lock[cab_idx]->setCombineCranePosition(vc_inputs.getValue(cab_idx, CTRL_COMBINE_KRAN));

        // Кнопки свистка, тифона
        horn[cab_idx]->setSvistokOn(vc_inputs.getBool(cab_idx, CTRL_WHISTLE_BUTTON) || vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_WHISTLE));
        horn[cab_idx]->setTifonOn(vc_inputs.getBool(cab_idx, CTRL_TIFON_BUTTON) || vc_inputs.getBool(cab_idx, CTRL_TUMBLER_P_TIFON));

        // Рукоятки бдительности
        vc_inputs.getBool(cab_idx, CTRL_RBS_BUTTON) ? rb[cab_idx][RBS].set() : rb[cab_idx][RBS].reset();
        vc_inputs.getBool(cab_idx, CTRL_RB_BUTTON)  ? rb[cab_idx][RB_1].set() : rb[cab_idx][RB_1].reset();
        vc_inputs.getBool(cab_idx, CTRL_RBP_BUTTON) ? rb[cab_idx][RBP].set() : rb[cab_idx][RBP].reset();

        // Автоведение и маневровый режим
        vc_inputs.getBool(cab_idx, CTRL_AUTOPILOT) ? autopilot_switcher[cab_idx].set() : autopilot_switcher[cab_idx].reset();
        vc_inputs.getBool(cab_idx, CTRL_SHUNTING_MODE) ? tumbler_shunting_mode[cab_idx].set() : tumbler_shunting_mode[cab_idx].reset();
    }

    // Концевые краны тормозной магистрали (общие, не привязаны к кабинам)
    const size_t shared_cab = vc_inputs.size() - 1;
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BP_FWD) ? anglecock_bp_fwd->open() : anglecock_bp_fwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BP_BWD) ? anglecock_bp_bwd->open() : anglecock_bp_bwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_FL_FWD) ? anglecock_fl_fwd->open() : anglecock_fl_fwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_FL_BWD) ? anglecock_fl_bwd->open() : anglecock_fl_bwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BC_FWD) ? anglecock_bc_fwd->open() : anglecock_bc_fwd->close();
    vc_inputs.getBool(shared_cab, CTRL_ANGLECOCK_BC_BWD) ? anglecock_bc_bwd->open() : anglecock_bc_bwd->close();    

    // Рукава магистралей (общие, не привязаны к кабинам)
    hose_bp_fwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BP_FWD))));
    hose_bp_bwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BP_BWD))));
    hose_fl_fwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_FL_FWD))));
    hose_fl_bwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_FL_BWD))));
    hose_bc_fwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BC_FWD))));
    hose_bc_bwd->setExternalState(static_cast<int>(std::round(vc_inputs.getValue(shared_cab, CTRL_HOSE_BC_BWD))));

    // Расцепные рычаги сцепных устройств (общие, не привязаны к кабинам)
    // Передняя сцепка
    if (vc_inputs.getBool(shared_cab, CTRL_OPER_ROD_FIX_FWD))
        oper_rod_fwd->setExternalState(-1.0);
    else
        oper_rod_fwd->setExternalState(vc_inputs.getValue(shared_cab, CTRL_OPER_ROD_FWD));

    // Задняя сцепка
    if (vc_inputs.getBool(shared_cab, CTRL_OPER_ROD_FIX_BWD))
        oper_rod_bwd->setExternalState(-1.0);
    else
        oper_rod_bwd->setExternalState(vc_inputs.getValue(shared_cab, CTRL_OPER_ROD_BWD));

    // Кнопки песочницы
    sand_system->setSandDeliveryOn(vc_inputs.getBool(CAB1, CTRL_SAND_BUTTON) || vc_inputs.getBool(CAB2, CTRL_SAND_BUTTON));
}