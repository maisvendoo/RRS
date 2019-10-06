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
{
    tracForce_kN = 0;
    bv_return = false;
    Uks = 3000;

    U_kr = 0;

    EDT = false;
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

    initOtherEquipment();

    initRegistrator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::step(double t, double dt)
{
    stepPantographs(t, dt);

    stepFastSwitch(t, dt);

    stepTractionControl(t, dt);

    stepProtection(t, dt);

    stepAirSupplySubsystem(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesMech(t , dt);

    stepBrakesEquipment(t, dt);

    stepEDT(t, dt);

    stepDebugMsg(t, dt);

    stepSignals();

    //registrate(t, dt);

    horn->setControl(keys);
    horn->step(t, dt);
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
void CHS2T::keyProcess()
{
    if (getKeyState(KEY_8))
    {
        if (isShift())
            mk_tumbler.set();

        else
            mk_tumbler.reset();
    }

    if (getKeyState(KEY_9))
    {
        if (isShift())
            EDTSwitch.set();
        else
            EDTSwitch.reset();
    }
}

GET_VEHICLE(CHS2T)
