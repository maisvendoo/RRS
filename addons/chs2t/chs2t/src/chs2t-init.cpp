#include    "chs2t.h"

#include    <QDir>

#include    "Journal.h"

//------------------------------------------------------------------------------
// Инициализация токоприемников
//------------------------------------------------------------------------------
void CHS2T::initPantographs()
{
    Journal::instance()->info("Init pantographs");

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(config_dir + QDir::separator() + "pantograph");
        pantographs[i]->setUks(Uks);
        connect(pantographs[i], &Pantograph::soundPlay, this, &CHS2T::soundPlay);
    }

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantoSwitcher[i] = new CHS2TSwitcher(Q_NULLPTR, 0, 4);
        pantoSwitcher[i]->setSpring(3,2);
        pantoSwitcher[i]->setSoundName("tumbler");
        connect(pantoSwitcher[i], &Switcher::soundPlay, this, &CHS2T::soundPlay);
    }

    pantoSwitcher[0]->setKeyCode(KEY_I);
    pantoSwitcher[1]->setKeyCode(KEY_O);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesMech()
{
    Journal::instance()->info("Init brake mechanics");

    brakesMech[0] = new CHS2tBrakeMech();
    brakesMech[0]->read_custom_config(config_dir + QDir::separator() + "brake-mech-front");

    brakesMech[1] = new CHS2tBrakeMech();
    brakesMech[1]->read_custom_config(config_dir + QDir::separator() + "brake-mech-back");
}

//------------------------------------------------------------------------------
// Инициализация быстродействующего выключателя
//------------------------------------------------------------------------------
void CHS2T::initFastSwitch()
{
    Journal::instance()->info("Init fast switch");

    bv = new ProtectiveDevice();
    bv->read_custom_config(config_dir + QDir::separator() + "bv");

    fastSwitchSw = new CHS2TSwitcher(Q_NULLPTR, KEY_P, 4);
    fastSwitchSw->setSpring(3, 2);
    fastSwitchSw->setSoundName("tumbler");
    connect(fastSwitchSw, &Switcher::soundPlay, this, &CHS2T::soundPlay);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initProtection()
{
    Journal::instance()->info("Init protection devices");

    overload_relay = new OverloadRelay();
    overload_relay->read_custom_config(config_dir + QDir::separator() + "1RPD6");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesControl(QString module_path)
{
    Journal::instance()->info("Init brake control devices");

    brakeCrane = loadBrakeCrane(module_path + QDir::separator() + "krm395");
    brakeCrane->read_config("krm395");
    connect(brakeCrane, &BrakeCrane::soundPlay, this, &CHS2T::soundPlay);
    connect(brakeCrane, &BrakeCrane::soundSetVolume, this, &CHS2T::soundSetVolume);

    locoCrane = loadLocoCrane(module_path + QDir::separator() + "kvt254");
    locoCrane->read_config("kvt254");
    connect(locoCrane, &LocoCrane::soundPlay, this, &CHS2T::soundPlay);
    connect(locoCrane, &LocoCrane::soundSetVolume, this, &CHS2T::soundSetVolume);
    connect(locoCrane, &LocoCrane::soundPlay, this, &CHS2T::soundPlay);

    handleEDT = new HandleEDT();
    handleEDT->read_custom_config(config_dir + QDir::separator() + "handle-edt");
    handleEDT->setBrakeKey(KEY_Period);
    handleEDT->setReleaseKey(KEY_Comma);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initAirSupplySubsystem()
{
    Journal::instance()->info("Init air suplly subsystem");

    mainReservoir = new Reservoir(1);
    spareReservoir = new Reservoir(0.078);
    brakeRefRes = new Reservoir(0.004);

    for (size_t i = 0; i < motor_compressor.size(); ++i)
    {
        motor_compressor[i] = new DCMotorCompressor();
        motor_compressor[i]->read_custom_config(config_dir + QDir::separator() + "motor-compressor");
        connect(motor_compressor[i], &DCMotorCompressor::soundPlay, this, &CHS2T::soundPlay);
        connect(motor_compressor[i], &DCMotorCompressor::soundStop, this, &CHS2T::soundStop);

        motor_compressor[i]->setSoundName(QString("kompressor%1").arg(i+1));

        mk_switcher[i] = new CHS2TSwitcher(Q_NULLPTR, 0, 4);
        mk_switcher[i]->setSoundName("tumbler");
        connect(mk_switcher[i], &Switcher::soundPlay, this, &CHS2T::soundPlay);
    }

    mk_switcher[0]->setKeyCode(KEY_7);
    mk_switcher[1]->setKeyCode(KEY_8);

    pressReg = new PressureRegulator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initTractionControl()
{
    Journal::instance()->info("Init traction control");

    km21KR2 = new Km21KR2();
    connect(km21KR2, &Km21KR2::soundPlay, this, &CHS2T::soundPlay);

    stepSwitch = new StepSwitch();
    stepSwitch->read_custom_config(config_dir + QDir::separator() + "step-switch");

    motor = new Motor();
    motor->setCustomConfigDir(config_dir);
    motor->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");
    connect(motor, &Motor::soundSetVolume, this, &CHS2T::soundSetVolume);
    connect(motor, &Motor::soundSetPitch, this, &CHS2T::soundSetPitch);

    puskRez = new PuskRez;
    puskRez->read_custom_config(config_dir + QDir::separator() + "puskrez");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesEquipment(QString module_path)
{
    Journal::instance()->info("Init brakes equipment");

    dako = new Dako();
    dako->read_custom_config(config_dir + QDir::separator() + "dako");

    electroAirDistr = loadElectroAirDistributor(module_path + QDir::separator() + "evr305");
    electroAirDistr->read_config("evr305");

    airDistr = loadAirDistributor(module_path + QDir::separator() + "vr242");
    airDistr->read_config("vr242");

    autoTrainStop = loadAutoTrainStop(module_path + QDir::separator() + "epk150");
    autoTrainStop->read_config("epk150");
    connect(autoTrainStop, &AutoTrainStop::soundPlay, this, &CHS2T::soundPlay);
    connect(autoTrainStop, &AutoTrainStop::soundStop, this, &CHS2T::soundStop);

    zpk = new SwitchingValve();
    zpk->read_config("zpk");

    rd304 = new PneumoReley();
    rd304->read_config("rd304");

    pnSplit = new PneumoSplitter();
    pnSplit->read_config("pneumo-splitter");

    airSplit = new PneumoSplitter();
    airSplit->read_config("pneumo-splitter");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initEDT()
{
    Journal::instance()->info("Init electrical brake system");

    generator = new Generator();
    generator->setCustomConfigDir(config_dir);
    generator->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");
    connect(generator, &Generator::soundSetPitch, this, &CHS2T::soundSetPitch);
    connect(generator, &Generator::soundSetVolume, this, &CHS2T::soundSetVolume);

    pulseConv = new PulseConverter();

    BrakeReg = new BrakeRegulator();
    BrakeReg->read_custom_config(config_dir + QDir::separator() + "brake-regulator");

    EDT_timer.setTimeout(3.0);
    EDT_timer.firstProcess(false);
    connect(&EDT_timer, &Timer::process, this, &CHS2T::enableEDT);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initOtherEquipment()
{
    Journal::instance()->info("Init whistle and typhoid");

    horn = new CHS2tHorn();
    connect(horn, &CHS2tHorn::soundPlay, this, &CHS2T::soundPlay);
    connect(horn, &CHS2tHorn::soundStop, this, &CHS2T::soundStop);

    speed_meter = new SL2M();
    speed_meter->read_custom_config(config_dir + QDir::separator() + "3SL-2M");
    connect(speed_meter, &SL2M::soundSetVolume, this, &CHS2T::soundSetVolume);

    safety_device = new SafetyDevice();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initSupportEquipment()
{
    Journal::instance()->info("Init support equipment");

    relValve = new ReleaseValve();
    relValve->read_custom_config(config_dir + QDir::separator() + "release-valve");

    motor_fan_ptr = new DCMotorFan();
    motor_fan_ptr->read_custom_config(config_dir + QDir::separator() + "dc-motor-fan");
    connect(motor_fan_ptr, &DCMotorFan::soundPlay, this, &CHS2T::soundPlay);
    connect(motor_fan_ptr, &DCMotorFan::soundStop, this, &CHS2T::soundStop);
    motor_fan_ptr->setSoundName("PTR_fan");

    for (size_t i = 0; i < motor_fan.size(); ++i)
    {
        motor_fan[i] = new DCMotorFan();
        motor_fan[i]->read_custom_config(config_dir + QDir::separator() + "motor-fan");
        connect(motor_fan[i], &DCMotorFan::soundPlay, this, &CHS2T::soundPlay);
        connect(motor_fan[i], &DCMotorFan::soundStop, this, &CHS2T::soundStop);
        motor_fan[i]->setSoundName(QString("Motor_Fan%1").arg(i+1));
    }

    motor_fan_switcher = new CHS2TSwitcher(Q_NULLPTR, KEY_F, 3);
    motor_fan_switcher->setSoundName("tumbler");
    connect(motor_fan_switcher, &Switcher::soundPlay, this, &CHS2T::soundPlay);

    connect(motor_fan_ptr, &DCMotorFan::soundSetPitch, this, &CHS2T::soundSetPitch);

    blinds = new Blinds();
    blinds->read_custom_config(config_dir + QDir::separator() + "blinds");

    blindsSwitcher = new CHS2TSwitcher(Q_NULLPTR, KEY_G, 5);
    blindsSwitcher->setSoundName("tumbler");
    connect(blindsSwitcher, &Switcher::soundPlay, this, &CHS2T::soundPlay);

    energy_counter = new EnergyCounter();
    energy_counter->read_custom_config(config_dir + QDir::separator() + "energy-counter");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initModbus()
{
    Journal::instance()->info("Init modbus");

    QString modbusCfgDir = config_dir + QDir::separator() + "modbus";

    TM_manometer = new PhysToModbus();
    TM_manometer->load((modbusCfgDir + QDir::separator() + "manometer-TM").toStdString());

    UR_manometer = new PhysToModbus();
    UR_manometer->load((modbusCfgDir + QDir::separator() + "manometer-UR").toStdString());

    ZT_manometer = new PhysToModbus();
    ZT_manometer->load((modbusCfgDir + QDir::separator() + "manometer-ZT").toStdString());

    GR_manometer = new PhysToModbus();
    GR_manometer->load((modbusCfgDir + QDir::separator() + "manometer-GR").toStdString());

    TC_manometer = new PhysToModbus();
    TC_manometer->load((modbusCfgDir + QDir::separator() + "manometer-TC").toStdString());

}

//------------------------------------------------------------------------------
// Инициализация регистратора
//------------------------------------------------------------------------------
void CHS2T::initRegistrator()
{
    Journal::instance()->info("Init registraion subsystem");

    reg = nullptr;
    //reg = new Registrator("motor", 1e-3, Q_NULLPTR);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakeDevices(double p0, double pTM, double pFL)
{
    Journal::instance()->info("Init brake devices: callback form TrainEngine");

    charging_press = p0;

    mainReservoir->setY(0, pFL);
    spareReservoir->setY(0, charging_press);
    brakeCrane->init(pTM, pFL);
    locoCrane->init(pTM, pFL);
    airDistr->init(pTM, pFL);
    autoTrainStop->init(pTM, pFL);
}

//------------------------------------------------------------------------------
// Инициализация списка звуков перестука
//------------------------------------------------------------------------------
void CHS2T::initTapSounds()
{
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
