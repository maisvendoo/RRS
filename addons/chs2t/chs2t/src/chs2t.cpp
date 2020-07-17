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
#include    "chs2t-signals.h"

#include    "filesystem.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
CHS2T::CHS2T() : Vehicle()
{
    eptSwitch.setOnSoundName("tumbler");
    eptSwitch.setOffSoundName("tumbler");
    connect(&eptSwitch, &Trigger::soundPlay, this, &CHS2T::soundPlay);

    U_bat = 55.0;

    tracForce_kN = 0;
    bv_return = false;
    Uks = 3000;

    U_kr = 0;

    EDT = false;    

    dropPosition = false;
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

    initBrakesMech();

    initFastSwitch();

    initProtection();

    initBrakesControl(modules_dir);

    initAirSupplySubsystem();

    initTractionControl();

    initBrakesEquipment(modules_dir);

    initEDT();

    initSupportEquipment();

    initOtherEquipment();

    initEPT();

    initModbus();

    //initRegistrator();

    initSounds();

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
    control_signals.analogSignal[999].cur_value = static_cast<float>(is_controlled);

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
    stepAirSupplySubsystem(t, dt);

    //Journal::instance()->info("Step brakes control");
    stepBrakesControl(t, dt);

    //Journal::instance()->info("Step brake mech");
    stepBrakesMech(t , dt);

    //Journal::instance()->info("Step brake equipment");
    stepBrakesEquipment(t, dt);

    //Journal::instance()->info("Step EDT");
    stepEDT(t, dt);
    stepEDT2(t, dt);

    //Journal::instance()->info("Step support equipment");
    stepSupportEquipment(t, dt);

    stepEPT(t, dt);

    //Journal::instance()->info("Step debug");
    stepDebugMsg(t, dt);

    //Journal::instance()->info("Step signals");
    stepSignals();

    stepSwitcherPanel();

    stepDecodeAlsn();

    stepTapSound();

    //registrate(t, dt);

    //Journal::instance()->info("Step horn");
    horn->setControl(keys, control_signals);
    horn->step(t, dt);

    //Journal::instance()->info("Step speed meter");
    speed_meter->setOmega(wheel_omega[0]);
    speed_meter->setWheelDiameter(wheel_diameter);
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

    }
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::hardwareOutput()
{
    feedback_signals.analogSignal[MAN_TM].cur_value = TM_manometer->getModbus(pTM);
    feedback_signals.analogSignal[MAN_UR].cur_value = UR_manometer->getModbus(brakeCrane->getEqReservoirPressure());
    feedback_signals.analogSignal[MAN_ZT].cur_value = ZT_manometer->getModbus(brakeRefRes->getPressure());
    feedback_signals.analogSignal[MAN_GR].cur_value = GR_manometer->getModbus(mainReservoir->getPressure());
    feedback_signals.analogSignal[MAN_TC].cur_value = TC_manometer->getModbus(brakesMech[0]->getBrakeCylinderPressure());

    feedback_signals.analogSignal[VOLT_BAT].cur_value = PtM_U_bat->getModbus(U_bat);
    feedback_signals.analogSignal[VOLT_EPT].cur_value = EPT_U->getModbus(ept_converter->getU_out());
    feedback_signals.analogSignal[VOLT_NETWORK].cur_value = Network_U->getModbus(U_kr / 1000.0);

    feedback_signals.analogSignal[AMPER_1_2].cur_value = Amper_12->getModbus(motor->getI12() / 1000.0);

    if (EDT)
    {
        feedback_signals.analogSignal[AMPER_3_4].cur_value = Amper_34->getModbus(abs(generator->getIf()) / 1000.0);
        feedback_signals.analogSignal[AMPER_5_6].cur_value = Amper_56->getModbus(abs(generator->getIa()) / 1000.0);
    }
    else
    {
        feedback_signals.analogSignal[AMPER_3_4].cur_value = Amper_34->getModbus(motor->getI34() / 1000.0);
        feedback_signals.analogSignal[AMPER_5_6].cur_value = Amper_56->getModbus(motor->getI56() / 1000.0);
    }

    feedback_signals.analogSignal[POS_INDICATOR].cur_value = Pos_Indicator->getModbus(stepSwitch->getPoz());

    feedback_signals.analogSignal[LAMP_P].cur_value = analogSignal[SIGLIGHT_P];
    feedback_signals.analogSignal[LAMP_SP].cur_value = analogSignal[SIGLIGHT_SP];
    feedback_signals.analogSignal[LAMP_S].cur_value = analogSignal[SIGLIGHT_S];
    feedback_signals.analogSignal[LAMP_ZERO].cur_value = analogSignal[SIGLIGHT_ZERO];

    feedback_signals.analogSignal[LAMP_R].cur_value = analogSignal[SIGLIGHT_R];
    feedback_signals.analogSignal[LAMP_T].cur_value = analogSignal[SIGLIGHT_T];
    feedback_signals.analogSignal[LAMP_PEREKRISHA].cur_value = analogSignal[SIGLIGHT_PEREKRISHA];
    feedback_signals.analogSignal[LAMP_O].cur_value = analogSignal[SIGLIGHT_O];

    feedback_signals.analogSignal[LAMP_NAR_SYNC].cur_value = analogSignal[SIGLIGHT_NAR_SYNC];
    feedback_signals.analogSignal[LAMP_NO_BRAKES_RELEASE].cur_value = analogSignal[SIGLIGHT_NO_BRAKES_RELEASE];
    feedback_signals.analogSignal[LAMP_PESOK].cur_value = analogSignal[SIGLIGHT_PESOK];
    feedback_signals.analogSignal[LAMP_ZASHITA].cur_value = analogSignal[SIGLIGHT_ZASHITA];

    feedback_signals.analogSignal[LAMP_GALYZI].cur_value = analogSignal[SIGLIGHT_GALYZI];
    feedback_signals.analogSignal[LAMP_MK].cur_value = analogSignal[SIGLIGHT_MK];
    feedback_signals.analogSignal[LAMP_ACCUM2].cur_value = analogSignal[SIGLIGHT_ACCUM2];
    feedback_signals.analogSignal[LAMP_ACCUM1].cur_value = analogSignal[SIGLIGHT_ACCUM1];

    feedback_signals.analogSignal[LAMP_RAZED].cur_value = analogSignal[SIGLIGHT_RAZED];

    feedback_signals.analogSignal[SV_LAMP_G].cur_value = analogSignal[LS_G];
    feedback_signals.analogSignal[SV_LAMP_R].cur_value = analogSignal[LS_R];
    feedback_signals.analogSignal[SV_LAMP_W].cur_value = analogSignal[LS_W];
    feedback_signals.analogSignal[SV_LAMP_Y].cur_value = analogSignal[LS_Y];
    feedback_signals.analogSignal[SV_LAMP_YR].cur_value = analogSignal[LS_YR];

    feedback_signals.analogSignal[INDICATOR_BV].cur_value = analogSignal[INDICATOR_BV];
}

GET_VEHICLE(CHS2T)
