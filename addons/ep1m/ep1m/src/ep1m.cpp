#include    "ep1m.h"

#include    "filesystem.h"

#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1m::EP1m(QObject *parent) : Vehicle (parent)
{
    analogSignal.resize(SIGNALS_NUM_TOTAL);

    pressed_keys_by_cabine.resize(CABS_NUM);
    pressed_keys_by_cabine.shrink_to_fit();

    Uks = 25000.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1m::~EP1m()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initialization()
{
    FileSystem& fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());
    QString custom_cfg_dir(fs.getVehiclesDir().c_str());
    custom_cfg_dir += fs.separator() + config_dir;

    // Инициализация сцепных устройств
    initCouplings(modules_dir, custom_cfg_dir);

    // Инициализация питания цепей управления
    initControlPower(modules_dir, custom_cfg_dir);

    // Инициализация цепей управления
    initControlCircuit(modules_dir, custom_cfg_dir);

    // Инициализация пульта управления в кабине
    initPanel(modules_dir, custom_cfg_dir);

    // Инициализация микропроцессорной системы управления и диагностики (МСУД)
    initMSUD(modules_dir, custom_cfg_dir);

    // Инициализация силовой схемы
    initPowerCircuit(modules_dir, custom_cfg_dir);

    // Инициализация системы подготовки сжатого воздуха
    initPneumoSupply(modules_dir, custom_cfg_dir);

    // Инициализация приборов управления тормозами
    initBrakesControl(modules_dir, custom_cfg_dir);

    // Инициализация приборов торможения
    initBrakesEquipment(modules_dir, custom_cfg_dir);

    // Инициализация ЭПТ
    initEPB(modules_dir, custom_cfg_dir);

    // Инициализация вспомогательных машин
    initAuxMachines(modules_dir, custom_cfg_dir);

    // Инициализация приборов безопасности
    initSafetyDevices(modules_dir, custom_cfg_dir);

    // Инициализация прочих устройств
    initOtherEquipment(modules_dir, custom_cfg_dir);

    // Инициализация управления
    initControl(modules_dir, custom_cfg_dir);

    // Инициализация регистратора
    if (is_Registrator_on)
        initRegistartor(modules_dir, custom_cfg_dir);

    connect(autoStartTimer, &Timer::process, this, &EP1m::slotAutostart);

    initAutopilot(modules_dir, custom_cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::process(const simulator_time_t& t, const double& dt)
{
    if (needDebugMsg)
        debugPrint(t, dt);

    keyProcess(t, dt);

    stepControls(t.simulation_seconds, dt);

    signalsOutput(t, dt);

    soundsOutput(t, dt);

    // Работа автоведения
    stepAutopilot(t.simulation_seconds, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::preStep(const double& t)
{
    preStepCouplings(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::step(const double& t, const double& dt)
{
    // Моделирование сцепных устройств
    stepCouplings(t, dt);

    // Работа подсистемы питания цепей управления
    stepControlPower(t, dt);

    // Работа цепей управления
    stepControlCircuit(t, dt);

    // Работа приборов управления в кабине
    stepPanel(t, dt);

    // Работа микропроцессорной системы управления и диагностики
    stepMSUD(t, dt);

    // Работа силовой схемы
    stepPowerCircuit(t, dt);

    // Работа системы подготовки сжатого воздуха
    stepPneumoSupply(t, dt);

    // Работа приборов управления тормозами
    stepBrakesControl(t, dt);

    // Работа приборов торможения
    stepBrakesEquipment(t, dt);

    // Работа ЭПТ
    stepEPB(t, dt);

    // Работа вспомогательных машин
    stepAuxMachines(t, dt);

    // Работа приборов безопасности
    stepSafetyDevices(t, dt);

    // Работа прочих устройств
    stepOtherEquipment(t, dt);

    // Регистрация параметров движения
    if (is_Registrator_on)
        stepRegistration(t, dt);

    autoStartTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_MODULE(EP1m)
