//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------

#include    "vl60pk.h"
#include    "vl60-signals.h"
#include    "filesystem.h"
#include    <core/get_module.h>

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60pk::VL60pk() : Vehicle ()
{
    analogSignal.resize(SIGNALS_NUM_TOTAL);

    pressed_keys_by_cabine.resize(CABS_NUM);
    pressed_keys_by_cabine.shrink_to_fit();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60pk::~VL60pk()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initialization()
{
    FileSystem& fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());
    QString custom_cfg_dir(fs.getVehiclesDir().c_str());
    custom_cfg_dir += fs.separator() + config_dir;

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

    initCouplings(modules_dir, custom_cfg_dir);

    initPantographs(modules_dir, custom_cfg_dir);

    initHighVoltageScheme(modules_dir, custom_cfg_dir);

    initSupplyMachines(modules_dir, custom_cfg_dir);

    initPneumoSupply(modules_dir, custom_cfg_dir);

    initBrakesControl(modules_dir, custom_cfg_dir);

    initBrakesEquipment(modules_dir, custom_cfg_dir);

    initEPB(modules_dir, custom_cfg_dir);

    initTractionControl(modules_dir, custom_cfg_dir);

    initSafetyDevices(modules_dir, custom_cfg_dir);

    initOtherEquipment(modules_dir, custom_cfg_dir);

    initControl(modules_dir, custom_cfg_dir);

    initAutopilot(modules_dir, custom_cfg_dir);

    autoStartTimer = new Timer(0.5, false);
    connect(autoStartTimer, &Timer::process, this, &VL60pk::slotAutoStart);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::process(const simulator_time_t& t, const double& dt)
{
    if (needDebugMsg)
        debugPrint(t, dt);

    keyProcess(t, dt);

    signalsOutput(t, dt);

    soundsOutput(t, dt);

    stepAutopilot(t.simulation_seconds, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::preStep(const double& t)
{
    preStepCouplings(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::step(const double &t, const double &dt)
{
    stepCouplings(t, dt);

    stepPantographsControl(t, dt);

    stepMainSwitchControl(t, dt);

    stepTracTransformer(t, dt);

    stepPhaseSplitter(t, dt);

    stepMotorFans(t, dt);

    stepPneumoSupply(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    stepTractionControl(t, dt);

    stepLineContactors(t, dt);

    stepOtherEquipment(t, dt);

    stepSafetyDevices(t, dt);

    stepControls(t, dt);

    autoStartTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_MODULE(VL60pk)
