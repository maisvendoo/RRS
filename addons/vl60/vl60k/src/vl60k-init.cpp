#include    "vl60k.h"

#include "automatic-train-stop.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "overload-relay.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "pneumo-brake-lock.h"
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
void VL60k::initPantographs(const QString& modules_dir, const QString& custom_cfg_dir)
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
void VL60k::initHighVoltageScheme(const QString& modules_dir, const QString& custom_cfg_dir)
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
void VL60k::initSupplyMachines(const QString& modules_dir, const QString& custom_cfg_dir)
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
void VL60k::initTractionControl(const QString& modules_dir, const QString& custom_cfg_dir)
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
void VL60k::initOtherEquipment(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void)modules_dir;
    (void)custom_cfg_dir;

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
    reg->setFileName("vl60k-motor");
    reg->init();
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60k::initAutostartProgram(int cab_autostart_request)
{
    if (autoStartTimer->isStarted())
        return false;

    if ((cab_autostart_request != CAB1) && (cab_autostart_request != CAB2))
        return false;

    if (controller[(cab_autostart_request == CAB1) ? CAB2 : CAB1]->isReversHandle())
        return false;

    if (!brake_lock[cab_autostart_request]->isLockHandleAllowed())
        return false;

    if (!epk[cab_autostart_request]->isKeyAllowed())
        return false;

    autostart_cab = cab_autostart_request;
    controller[autostart_cab]->insertReversHandle(true);
    brake_lock[autostart_cab]->setStateOn(true);
    epk[autostart_cab]->insertKey(true);

    controller[CAB1]->setControl();
    controller[CAB2]->setControl();
    brake_lock[CAB1]->setControl();
    brake_lock[CAB2]->setControl();
    epk[CAB1]->setControl();
    epk[CAB2]->setControl();

    start_count = 0;
    buildAutostartTriggers(autostart_cab);

    autostart_mode = AUTOSTART_ON;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::buildAutostartTriggers(int cab)
{
    triggers.clear();
    triggers.reserve(15);
    triggers.push_back(&pants_tumbler[cab]);
    triggers.push_back(&pant2_tumbler[cab]);
    triggers.push_back(&gv_tumbler[cab]);
    triggers.push_back(&gv_return_tumbler[cab]);
    triggers.push_back(&fr_tumbler[cab]);
    triggers.push_back(&mk_tumbler[cab]);

    for (size_t i = 0; i < NUM_MOTOR_FANS; ++i)
        triggers.push_back(&mv_tumblers[cab][i]);

    triggers.push_back(&cu_tumbler[cab]);

    if (!epk[cab]->isKeyOn())
        triggers.push_back(&rb[cab][RBS]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60k::initAutostopProgram(int cab_autostop_request)
{
    if (autoStartTimer->isStarted())
        return false;

    if ((cab_autostop_request != CAB1) && (cab_autostop_request != CAB2))
        return false;

    // Во второй кабине не должно быть реверсивной рукоятки
    if (controller[(cab_autostop_request == CAB1) ? CAB2 : CAB1]->isReversHandle())
        return false;

    // Реверсивная рукоятка должна быть в запрашиваемой кабине
    if (!controller[cab_autostop_request]->isReversHandle())
        return false;

    autostart_cab = cab_autostop_request;

    controller[CAB1]->setControl();
    controller[CAB2]->setControl();
    brake_lock[CAB1]->setControl();
    brake_lock[CAB2]->setControl();
    epk[CAB1]->setControl();
    epk[CAB2]->setControl();

    start_count = 0;
    buildAutostartTriggers(autostart_cab);

    autostart_mode = AUTOSTART_OFF;

    return true;
}
