#include    "vl60pk.h"

#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "overload-relay.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "protective-device.h"
#include "rectifier.h"
#include "relay.h"
#include "sanding-system.h"
#include "spotlight.h"
#include "trac-transformer.h"
#include "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initPantographs(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_config("pantograph", custom_cfg_dir);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initHighVoltageScheme(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    main_switch = new ProtectiveDevice();
    main_switch->read_config("main-switch", custom_cfg_dir);

    gauge_KV_ks = new Oscillator();
    gauge_KV_ks->read_config("oscillator");

    trac_trans = new TracTransformer();
    trac_trans->read_config("trac-transformer", custom_cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initSupplyMachines(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    phase_spliter = new PhaseSplitter();
    phase_spliter->read_config("phase-splitter", custom_cfg_dir);

    for (size_t i = 0; i < motor_fans.size(); ++i)
    {
        motor_fans[i] = new ACMotorFan();
        motor_fans[i]->read_config("motor-fan-ac");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initTractionControl(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    for (size_t i : {CAB1, CAB2})
    {
        controller[i] = new ControllerKME_60_044();
    }

    main_controller = new EKG_8G();
    main_controller->read_config("ekg-8g", custom_cfg_dir);

    for (size_t i = 0; i < vu.size(); ++i)
    {
        vu[i] = new Rectifier();
        vu[i]->read_config("VU", custom_cfg_dir);
    }

    gauge_KV_motors = new Oscillator();
    gauge_KV_motors->read_config("KV1-osc", custom_cfg_dir);

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i] = new DCMotor();
        motor[i]->setCustomConfigDir(config_dir);
        motor[i]->read_config("HB-412K", custom_cfg_dir);

        overload_relay[i] = new OverloadRelay();
        overload_relay[i]->read_config("PT-140A", custom_cfg_dir);

        linear_contactor[i] = new Relay(NUM_LC_CONTACTS);
        linear_contactor[i]->read_config("bv-8", custom_cfg_dir);
        linear_contactor[i]->setInitContactState(LC_SELF, false);
        linear_contactor[i]->setInitContactState(LC_TED, false);
        linear_contactor[i]->setInitContactState(LC_TED_LAMP, true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initOtherEquipment(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    for (auto i : {CAB1, CAB2})
    {
        horn[i] = new TrainHorn();
        horn[i]->read_config("train-horn");
    }

    // Система подачи песка
    sand_system = new SandingSystem();
    sand_system->read_config("sanding-system");
    sand_system->setSandMassMax(payload_mass);
    sand_system->setSandLevel(payload_coeff);

    // Прожекторы
    for (auto i : {CAB1, CAB2})
    {
        spotlight[i] = new SpotLight();
        spotlight[i]->read_config("spotlight");
    }
/*
    reg = new Registrator();
    reg->setFileName("vl60pk-motor");
    reg->init();
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initTriggers(int cab_autostart_request)
{
    if (autoStartTimer->isStarted())
        return;

    if ((cab_autostart_request == CAB1) && (controller[CAB2]->isReversHandle()))
        return;

    if ((cab_autostart_request == CAB2) && (controller[CAB1]->isReversHandle()))
        return;

    if ((cab_autostart_request != CAB1) && (cab_autostart_request != CAB2))
        return;

    controller[autostart_cab]->insertReversHandle(true);

    autostart_cab = cab_autostart_request;
    start_count = 0;
    triggers.clear();
    triggers.push_back(&pants_tumbler[autostart_cab]);
    triggers.push_back(&pant2_tumbler[autostart_cab]);
    triggers.push_back(&gv_tumbler[autostart_cab]);
    triggers.push_back(&gv_return_tumbler[autostart_cab]);
    triggers.push_back(&fr_tumbler[autostart_cab]);
    triggers.push_back(&mk_tumbler[autostart_cab]);

    for (size_t i = 0; i < NUM_MOTOR_FANS; ++i)
        triggers.push_back(&mv_tumblers[autostart_cab][i]);

    triggers.push_back(&cu_tumbler[autostart_cab]);
    triggers.push_back(&key_epk[autostart_cab]);
    triggers.push_back(&rb[autostart_cab][RBS]);
}
