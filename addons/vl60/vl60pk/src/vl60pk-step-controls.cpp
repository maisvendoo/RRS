#include    <vl60pk.h>
#include    <vl60-controls.h>
#include    <kme-60-044.h>
#include    <automatic-train-stop.h>
#include    <pneumo-brake-lock.h>
#include    <brake-crane.h>
#include    <loco-crane.h>
#include    <sanding-system.h>
#include    <train-horn.h>
#include    <pneumo-anglecock.h>
#include    <pneumo-hose-epb.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepControls(const double &t, const double &dt)
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

        epb_switch[cab_idx].step();
        autopilot_switcher[cab_idx].step();
        tumbler_shunting_mode[cab_idx].step();

        rb[cab_idx][RB_1].step();
        rb[cab_idx][RBS].step();
        rb[cab_idx][RBP].step();

        // Отсекаем управление при автозапуске или автоостанове
        if (autoStartTimer->isStarted())
        {
            continue;
        }

        // Автозапуск по нажатию комбинации клавиш в текущей кабине
        if (control_inputs[cab_idx][CTRL_AUTOSTART_PROGRAM].toBool())
        {
            if (initAutostartProgram(cab_idx))
            {
                autoStartTimer->start();
            }

            initClientInputSignal(cab_idx, CTRL_AUTOSTART_PROGRAM, 0.0f);

            return;
        }

        // Автоостанов по нажатию комбинации клавиш в текущей кабине
        if (control_inputs[cab_idx][CTRL_AUTOSTOP_PROGRAM].toBool())
        {
            if (initAutostopProgram(cab_idx))
            {
                autoStartTimer->start();
            }

            initClientInputSignal(cab_idx, CTRL_AUTOSTOP_PROGRAM, 0.0f);

            return;
        }

        bool is_pnt = control_inputs[cab_idx][CTRL_TUMBLER_PNT].toBool();
        is_pnt ? pants_tumbler[cab_idx].set() : pants_tumbler[cab_idx].reset();

        bool is_pnt1 = control_inputs[cab_idx][CTRL_TUMBLER_PNT1].toBool();
        is_pnt1 ? pant1_tumbler[cab_idx].set() : pant1_tumbler[cab_idx].reset();

        bool is_pnt2 = control_inputs[cab_idx][CTRL_TUMBLER_PNT2].toBool();
        is_pnt2 ? pant2_tumbler[cab_idx].set() : pant2_tumbler[cab_idx].reset();

        bool is_main_switch_on = control_inputs[cab_idx][CTRL_MAIN_SWITCH_ON].toBool();
        is_main_switch_on ? gv_tumbler[cab_idx].set() : gv_tumbler[cab_idx].reset();

        bool is_main_switch_return = control_inputs[cab_idx][CTRL_RETURN_PROTECTION].toBool();
        is_main_switch_return ? gv_return_tumbler[cab_idx].set() : gv_return_tumbler[cab_idx].reset();

        bool is_fr = control_inputs[cab_idx][CTRL_TUMBLER_FR].toBool();
        is_fr ? fr_tumbler[cab_idx].set() : fr_tumbler[cab_idx].reset();

        bool is_mk = control_inputs[cab_idx][CTRL_TUMBLER_MK].toBool();
        is_mk ? mk_tumbler[cab_idx].set() : mk_tumbler[cab_idx].reset();

        bool is_mv1 = control_inputs[cab_idx][CTRL_TUMBLER_MV1].toBool();
        is_mv1 ? mv_tumblers[cab_idx][MV1].set() : mv_tumblers[cab_idx][MV1].reset();

        bool is_mv2 = control_inputs[cab_idx][CTRL_TUMBLER_MV2].toBool();
        is_mv2 ? mv_tumblers[cab_idx][MV2].set() : mv_tumblers[cab_idx][MV2].reset();

        bool is_mv3 = control_inputs[cab_idx][CTRL_TUMBLER_MV3].toBool();
        is_mv3 ? mv_tumblers[cab_idx][MV3].set() : mv_tumblers[cab_idx][MV3].reset();

        bool is_mv4 = control_inputs[cab_idx][CTRL_TUMBLER_MV4].toBool();
        is_mv4 ? mv_tumblers[cab_idx][MV4].set() : mv_tumblers[cab_idx][MV4].reset();

        bool is_mv5 = control_inputs[cab_idx][CTRL_TUMBLER_MV5].toBool();
        is_mv5 ? mv_tumblers[cab_idx][MV5].set() : mv_tumblers[cab_idx][MV5].reset();

        bool is_mv6 = control_inputs[cab_idx][CTRL_TUMBLER_MV6].toBool();
        is_mv6 ? mv_tumblers[cab_idx][MV6].set() : mv_tumblers[cab_idx][MV6].reset();

        bool is_cu = control_inputs[cab_idx][CTRL_TUMBLER_CU].toBool();
        is_cu ? cu_tumbler[cab_idx].set() : cu_tumbler[cab_idx].reset();

        bool is_spot_high = control_inputs[cab_idx][CTRL_TUMBLER_SPOT_HIGH].toBool();
        is_spot_high ? spotlight_high_tumbler[cab_idx].set() : spotlight_high_tumbler[cab_idx].reset();

        bool is_spot_low = control_inputs[cab_idx][CTRL_TUMBLER_SPOT_LOW].toBool();
        is_spot_low ? spotlight_low_tumbler[cab_idx].set() : spotlight_low_tumbler[cab_idx].reset();

        bool is_cab_light_low = control_inputs[cab_idx][CTRL_TUMBLER_CAB_LIGHT_LOW].toBool();
        is_cab_light_low ? P_cab_light_low_tumbler[cab_idx].set() : P_cab_light_low_tumbler[cab_idx].reset();

        bool is_cab_light_high = control_inputs[cab_idx][CTRL_TUMBLER_CAB_LIGHT_HIGH].toBool();
        is_cab_light_high ? P_cab_light_high_tumbler[cab_idx].set() : P_cab_light_high_tumbler[cab_idx].reset();

        bool is_light_devices = control_inputs[cab_idx][CTRL_TUMBLER_LIGHT_DEVICES].toBool();
        is_light_devices ? P_light_devices_tumbler[cab_idx].set() : P_light_devices_tumbler[cab_idx].reset();

        bool is_buf_light_l = control_inputs[cab_idx][CTRL_TUMBLER_BUF_LIGHT_L].toBool();
        is_buf_light_l ? P_bufferlight_L_tumbler[cab_idx].set() : P_bufferlight_L_tumbler[cab_idx].reset();

        bool is_buf_light_r = control_inputs[cab_idx][CTRL_TUMBLER_BUF_LIGHT_R].toBool();
        is_buf_light_r ? P_bufferlight_R_tumbler[cab_idx].set() : P_bufferlight_R_tumbler[cab_idx].reset();

        bool is_buf_color_l = control_inputs[cab_idx][CTRL_TUMBLER_BUF_COLOR_L].toBool();
        is_buf_color_l ? P_buffercolor_L_toogle[cab_idx].set() : P_buffercolor_L_toogle[cab_idx].reset();

        bool is_buf_color_r = control_inputs[cab_idx][CTRL_TUMBLER_BUF_COLOR_R].toBool();
        is_buf_color_r ? P_buffercolor_R_toogle[cab_idx].set() : P_buffercolor_R_toogle[cab_idx].reset();

        bool is_epb = control_inputs[cab_idx][CTRL_TUMBLER_EPB].toBool();
        is_epb ? epb_switch[cab_idx].set() : epb_switch[cab_idx].reset();

        // Управление контроллером машиниста
        controller[cab_idx]->insertReversHandle(control_inputs[cab_idx][CTRL_REVERS_INSERTION].toBool());
        controller[cab_idx]->setReversHandlePos(control_inputs[cab_idx][CTRL_REVERS_POSITION].value);
        controller[cab_idx]->setMainHandlePos(control_inputs[cab_idx][CTRL_KM_MAIN_POSITION].value);

        // Управление ЭПК
        bool is_epk_insert = control_inputs[cab_idx][CTRL_EPK_INSERTION].toBool();
        epk[cab_idx]->insertKey(is_epk_insert);

        bool is_key_epk_ON = control_inputs[cab_idx][CTRL_KEY_EPK].toBool();
        epk[cab_idx]->setKeyOn(is_key_epk_ON);

        // Управление блокировкой 367
        bool is_lock367_insert = control_inputs[cab_idx][CTRL_LOCK_367_INSERTION].toBool();
        brake_lock[cab_idx]->setStateOn(is_lock367_insert);
        brake_lock[cab_idx]->insertLockHandle(is_lock367_insert);
        brake_lock[cab_idx]->setCombineCranePosition(control_inputs[cab_idx][CTRL_COMBINE_KRAN].value);

        // Кран 395
        brake_crane[cab_idx]->setHandlePosition(control_inputs[cab_idx][CTRL_KRM_395].value);

        // Кран 254
        loco_crane[cab_idx]->setHandlePosition(control_inputs[cab_idx][CTRL_KVT_254].value);

        // Кнопки свистка, тифона
        horn[cab_idx]->setSvistokOn(control_inputs[cab_idx][CTRL_WHISTLE_BUTTON].toBool());
        horn[cab_idx]->setTifonOn(control_inputs[cab_idx][CTRL_TIFON_BUTTON].toBool());

        // Рукоятки бдительности
        control_inputs[cab_idx][CTRL_RBS_BUTTON].toBool() ? rb[cab_idx][RBS].set() : rb[cab_idx][RBS].reset();
        control_inputs[cab_idx][CTRL_RB_BUTTON].toBool()  ? rb[cab_idx][RB_1].set() : rb[cab_idx][RB_1].reset();
        control_inputs[cab_idx][CTRL_RBP_BUTTON].toBool() ? rb[cab_idx][RBP].set() : rb[cab_idx][RBP].reset();

        // Автоведение и маневровый режим
        control_inputs[cab_idx][CTRL_AUTOPILOT].toBool() ? autopilot_switcher[cab_idx].set() : autopilot_switcher[cab_idx].reset();
        control_inputs[cab_idx][CTRL_SHUNTING_MODE].toBool() ? tumbler_shunting_mode[cab_idx].set() : tumbler_shunting_mode[cab_idx].reset();
    }

    // Концевые краны тормозной магистрали (общие, не привязаны к кабинам)
    auto &shared_inputs = control_inputs[control_inputs.size() - 1];
    shared_inputs[CTRL_ANGLECOCK_BP_FWD].toBool() ? anglecock_bp_fwd->open() : anglecock_bp_fwd->close();
    shared_inputs[CTRL_ANGLECOCK_BP_BWD].toBool() ? anglecock_bp_bwd->open() : anglecock_bp_bwd->close();
    shared_inputs[CTRL_ANGLECOCK_FL_FWD].toBool() ? anglecock_fl_fwd->open() : anglecock_fl_fwd->close();
    shared_inputs[CTRL_ANGLECOCK_FL_BWD].toBool() ? anglecock_fl_bwd->open() : anglecock_fl_bwd->close();
    shared_inputs[CTRL_ANGLECOCK_BC_FWD].toBool() ? anglecock_bc_fwd->open() : anglecock_bc_fwd->close();
    shared_inputs[CTRL_ANGLECOCK_BC_BWD].toBool() ? anglecock_bc_bwd->open() : anglecock_bc_bwd->close();

    // Синхронизация: отслеживание самопроизвольного рассоединения рукавов
    {
        static std::map<int, bool> prev_connected;
        int last_cab = control_inputs.size() - 1;

        auto check_hose = [&](int id, PneumoHose* hose) {
            bool now = hose->isConnected();
            if (prev_connected[id] && !now && shared_inputs[id].toBool())
            {
                shared_inputs[id].value = 0.0f;
                initClientInputSignal(last_cab, id, 0.0f);
            }
            prev_connected[id] = now;
        };

        check_hose(CTRL_HOSE_BP_FWD, hose_bp_fwd);
        check_hose(CTRL_HOSE_BP_BWD, hose_bp_bwd);
        check_hose(CTRL_HOSE_FL_FWD, hose_fl_fwd);
        check_hose(CTRL_HOSE_FL_BWD, hose_fl_bwd);
        check_hose(CTRL_HOSE_BC_FWD, hose_bc_fwd);
        check_hose(CTRL_HOSE_BC_BWD, hose_bc_bwd);
    }

    // Рукава магистралей (общие, не привязаны к кабинам)
    Journal::instance()->info(QString("DBG: Hose BP_FWD ctrl=%1").arg(shared_inputs[CTRL_HOSE_BP_FWD].value));
    shared_inputs[CTRL_HOSE_BP_FWD].toBool() ? hose_bp_fwd->connect() : hose_bp_fwd->disconnect();
    Journal::instance()->info(QString("DBG: Hose BP_BWD ctrl=%1").arg(shared_inputs[CTRL_HOSE_BP_BWD].value));
    shared_inputs[CTRL_HOSE_BP_BWD].toBool() ? hose_bp_bwd->connect() : hose_bp_bwd->disconnect();
    shared_inputs[CTRL_HOSE_FL_FWD].toBool() ? hose_fl_fwd->connect() : hose_fl_fwd->disconnect();
    shared_inputs[CTRL_HOSE_FL_BWD].toBool() ? hose_fl_bwd->connect() : hose_fl_bwd->disconnect();
    shared_inputs[CTRL_HOSE_BC_FWD].toBool() ? hose_bc_fwd->connect() : hose_bc_fwd->disconnect();
    shared_inputs[CTRL_HOSE_BC_BWD].toBool() ? hose_bc_bwd->connect() : hose_bc_bwd->disconnect();
    shared_inputs[CTRL_HOSE_BP_BWD].toBool() ? hose_bp_bwd->connect() : hose_bp_bwd->disconnect();
    shared_inputs[CTRL_HOSE_FL_FWD].toBool() ? hose_fl_fwd->connect() : hose_fl_fwd->disconnect();
    shared_inputs[CTRL_HOSE_FL_BWD].toBool() ? hose_fl_bwd->connect() : hose_fl_bwd->disconnect();
    shared_inputs[CTRL_HOSE_BC_FWD].toBool() ? hose_bc_fwd->connect() : hose_bc_fwd->disconnect();
    shared_inputs[CTRL_HOSE_BC_BWD].toBool() ? hose_bc_bwd->connect() : hose_bc_bwd->disconnect();

    // Кнопки песочницы
    sand_system->setSandDeliveryOn(control_inputs[CAB1][CTRL_SAND_BUTTON].toBool() || control_inputs[CAB2][CTRL_SAND_BUTTON].toBool());
}
