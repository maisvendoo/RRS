//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------

#include    "chs2t.h"

#include    "filesystem.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS2T::CHS2T() : Vehicle()
  , brake_crane_module_name("krm395")
  , brake_crane_config_name("krm395")
  , loco_crane_module_name("kvt254")
  , loco_crane_config_name("kvt254")
  , airdist_module_name("vr292")
  , airdist_config_name("vr292")
  , electro_airdist_module_name("evr305")
  , electro_airdist_config_name("evr305")
{
    epb_switch.setOnSoundName("tumbler");
    epb_switch.setOffSoundName("tumbler");
    connect(&epb_switch, &Trigger::soundPlay, this, &CHS2T::soundPlay);

    EDTSwitch.setOnSoundName("tumbler");
    EDTSwitch.setOffSoundName("tumbler");
    connect(&EDTSwitch, &Trigger::soundPlay, this, &CHS2T::soundPlay);

    U_bat = 55.0;

    tracForce_kN = 0;
    bv_return = false;
    Uks = 3000;

    U_kr = 0;

    EDT = false;

    dropPosition = false;

    state_RB = false;

    state_RBS = false;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
CHS2T::~CHS2T()
{

}

//------------------------------------------------------------------------------
// Общая инициализация локомотива
//------------------------------------------------------------------------------
void CHS2T::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

    Journal::instance()->info("Started DC electrical locomotive CS2t initialization...");

    initPantographs();

    initFastSwitch();

    initProtection();

    initPneumoSupply(modules_dir);

    initBrakesControl(modules_dir);

    initBrakesEquipment(modules_dir);

    initEPB(modules_dir);

    initTractionControl();

    initEDT();

    initSupportEquipment();

    initOtherEquipment();

    initModbus();

    initRegistrator();

    initTapSounds();

    for (size_t i = SWP1_POWER_1; i <= SWP1_POWER_10; ++i)
        feedback_signals.analogSignal[i].cur_value = 1;

    for (size_t i = SWP2_POWER_1; i <= SWP2_POWER_10; ++i)
        feedback_signals.analogSignal[i].cur_value = 1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    //Journal::instance()->info("Step pantographs");
    stepPantographs(t, dt);

    //Journal::instance()->info("Step fast switch");
    stepFastSwitch(t, dt);

    //Journal::instance()->info("Step traction control");
    stepTractionControl(t, dt);

    //Journal::instance()->info("Step protection");
    stepProtection(t, dt);

    //Journal::instance()->info("Step air supply");
    stepPneumoSupply(t, dt);

    //Journal::instance()->info("Step brakes control");
    stepBrakesControl(t, dt);

    //Journal::instance()->info("Step electropneumatic brakes");
    stepEPB(t, dt);

    //Journal::instance()->info("Step brake equipment");
    stepBrakesEquipment(t, dt);

    //Journal::instance()->info("Step EDT");
    stepEDT(t, dt);
    stepEDT2(t, dt);

    //Journal::instance()->info("Step support equipment");
    stepSupportEquipment(t, dt);

    //Journal::instance()->info("Step debug");
    stepDebugMsg(t, dt);

    //Journal::instance()->info("Step signals");
    stepSignals();

    stepSwitcherPanel();

    stepDecodeAlsn();

    stepTapSound();

    registrate(t, dt);

    //Journal::instance()->info("Step horn");
    horn->setControl(keys, control_signals);
    horn->step(t, dt);

    //Journal::instance()->info("Step speed meter");
    speed_meter->setOmega(wheel_omega[0]);
    speed_meter->setWheelDiameter(wheel_diameter[0]);
    speed_meter->step(t, dt);
}


//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void CHS2T::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getString(secName, "BrakeCraneModule", brake_crane_module_name);
        cfg.getString(secName, "BrakeCraneConfig", brake_crane_config_name);
        cfg.getString(secName, "LocoCraneModule", loco_crane_module_name);
        cfg.getString(secName, "LocoCraneConfig", loco_crane_config_name);
        cfg.getString(secName, "AirDistModule", airdist_module_name);
        cfg.getString(secName, "AirDistConfig", airdist_config_name);
        cfg.getString(secName, "ElectroAirDistModule", electro_airdist_module_name);
        cfg.getString(secName, "ElectroAirDistConfig", electro_airdist_config_name);
    }
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::hardwareOutput()
{
    feedback_signals.analogSignal[0].cur_value = TM_manometer->getModbus(brakepipe->getPressure());
    feedback_signals.analogSignal[1].cur_value = UR_manometer->getModbus(brake_crane->getERpressure());
    feedback_signals.analogSignal[2].cur_value = ZT_manometer->getModbus(brake_ref_res->getPressure());
    feedback_signals.analogSignal[3].cur_value = GR_manometer->getModbus(main_reservoir->getPressure());
    feedback_signals.analogSignal[4].cur_value = TC_manometer->getModbus(brake_mech[0]->getBCpressure());
}

GET_VEHICLE(CHS2T)
