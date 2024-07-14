#include    "vl60pk.h"

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
    connect(trac_trans, &TracTransformer::soundSetVolume, this, &VL60pk::soundSetVolume);
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

    controller = new ControllerKME_60_044();
    connect(controller, &ControllerKME_60_044::soundPlay, this, &VL60pk::soundPlay);

    main_controller = new EKG_8G();
    main_controller->read_config("ekg-8g", custom_cfg_dir);
    connect(main_controller, &EKG_8G::soundPlay, this, &VL60pk::soundPlay);

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
        connect(motor[i], &DCMotor::soundSetPitch, this, &VL60pk::soundSetPitch);
        connect(motor[i], &DCMotor::soundSetVolume, this, &VL60pk::soundSetVolume);

        overload_relay[i] = new OverloadRelay();
        overload_relay[i]->read_config("PT-140A", custom_cfg_dir);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initOtherEquipment(const QString &modules_dir, const QString &custom_cfg_dir)
{
    (void) modules_dir;

    speed_meter = new SL2M();
    speed_meter->setWheelDiameter(wheel_diameter[0]);
    speed_meter->read_config("3SL-2M", custom_cfg_dir);
    connect(speed_meter, &SL2M::soundSetVolume, this, &VL60pk::soundSetVolume);

    horn = new TrainHorn();
    horn->read_config("train-horn");

    // Система подачи песка
    sand_system = new SandingSystem();
    sand_system->read_config("sanding-system");
    sand_system->setSandMassMax(payload_mass);
    sand_system->setSandLevel(payload_coeff);
/*
    reg = new Registrator();
    reg->setFileName("vl60pk-step");
    reg->init();
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initTriggers()
{
    triggers.push_back(&pants_tumbler);
    triggers.push_back(&pant2_tumbler);
    triggers.push_back(&gv_tumbler);
    triggers.push_back(&gv_return_tumbler);
    triggers.push_back(&fr_tumbler);
    triggers.push_back(&mk_tumbler);

    for (size_t i = 0; i < mv_tumblers.size(); ++i)
        triggers.push_back(&mv_tumblers[i]);

    triggers.push_back(&cu_tumbler);

    autoStartTimer = new Timer(0.5);
    connect(autoStartTimer, &Timer::process, this, &VL60pk::slotAutoStart);
    start_count = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initTapSounds() {
    QString f_p = "tap_";

    tap_sounds << (f_p + "5-10");
    tap_sounds << (f_p + "10-20");
    tap_sounds << (f_p + "20-30");
    tap_sounds << (f_p + "30-40");
    tap_sounds << (f_p + "40-50");
    tap_sounds << (f_p + "50-60");
    tap_sounds << (f_p + "60-70");
    tap_sounds << (f_p + "70-80");
    tap_sounds << (f_p + "80-90");
    tap_sounds << (f_p + "90-100");
    tap_sounds << (f_p + "100-110");
    tap_sounds << (f_p + "110-~");
}
