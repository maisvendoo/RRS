#include "chs2t.h"

#include <QDir>

//------------------------------------------------------------------------------
// Инициализация токоприемников
//------------------------------------------------------------------------------
void CHS2T::initPantographs()
{
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph();
        pantographs[i]->read_custom_config(config_dir + QDir::separator() + "pantograph");
        pantographs[i]->setUks(Uks);
        connect(pantographs[i], &Pantograph::soundPlay, this, &CHS2T::soundPlay);
    }

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantoSwitcher[i] = new Switcher();
        pantoSwitcher[i]->setKolStates(4);
    }

    pantoSwitcher[0]->setKeyCode(KEY_I);
    pantoSwitcher[1]->setKeyCode(KEY_O);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesMech()
{
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
    bistV = new ProtectiveDevice();
    bistV->read_custom_config(config_dir + QDir::separator() + "bv");

    fastSwitchSw = new Switcher();
    fastSwitchSw->setKeyCode(KEY_P);
    fastSwitchSw->setKolStates(4);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initProtection()
{
    overload_relay = new OverloadRelay();
    overload_relay->read_custom_config(config_dir + QDir::separator() + "1RPD6");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesControl(QString module_path)
{
    brakeCrane = loadBrakeCrane(module_path + QDir::separator() + "krm395");
    brakeCrane->read_config("krm395");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initAirSupplySubsystem()
{
    mainReservoir = new Reservoir(1);
    spareReservoir = new Reservoir(0.078);
    brakeRefRes = new Reservoir(0.004);

    motor_compressor = new DCMotorCompressor();
    motor_compressor->read_custom_config(config_dir + QDir::separator() + "motor-compressor");
    pressReg = new PressureRegulator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initTractionControl()
{
    km21KR2 = new Km21KR2();

    stepSwitch = new StepSwitch();
    stepSwitch->read_custom_config(config_dir + QDir::separator() + "step-switch");

    motor = new Motor();
    motor->setCustomConfigDir(config_dir);
    motor->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");

    puskRez = new PuskRez;
    puskRez->read_custom_config(config_dir + QDir::separator() + "puskrez");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakesEquipment(QString module_path)
{
    dako = new Dako();
    dako->read_custom_config(config_dir + QDir::separator() + "dako");

    airDistr = loadAirDistributor(module_path + QDir::separator() + "vr242");
    airDistr->read_config("vr242");

    locoCrane = loadLocoCrane(module_path + QDir::separator() + "kvt254");
    locoCrane->read_config("kvt254");

    zpk = new SwitchingValve();
    zpk->read_config("zpk");

    rd304 = new PneumoReley();
    rd304->read_config("rd304");

    pnSplit = new PneumoSplitter();
    pnSplit->read_config("pneumo-splitter");

    airSplit = new PneumoSplitter();
    airSplit->read_config("pneumo-splitter");
}

void CHS2T::initEDT()
{
    generator = new Generator();
    generator->setCustomConfigDir(config_dir);
    generator->read_custom_config(config_dir + QDir::separator() + "AL-4846dT");

    pulseConv = new PulseConverter();

    BrakeReg = new BrakeRegulator();


}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initOtherEquipment()
{
    horn = new TrainHorn();
    connect(horn, &TrainHorn::soundSetVolume, this, &CHS2T::soundSetVolume);
}

//------------------------------------------------------------------------------
// Инициализация регистратора
//------------------------------------------------------------------------------
void CHS2T::initRegistrator()
{
    //reg = new Registrator("trackforce", 0.1, Q_NULLPTR);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::initBrakeDevices(double p0, double pTM, double pFL)
{
    charging_press = p0;

    mainReservoir->setY(0, pFL);
    spareReservoir->setY(0, charging_press);
    brakeCrane->init(pTM, pFL);
    locoCrane->init(pTM, pFL);
    airDistr->init(pTM, pFL);
}
