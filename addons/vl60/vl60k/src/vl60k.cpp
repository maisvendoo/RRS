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

#include    "vl60k.h"
#include    "filesystem.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60k::VL60k() : Vehicle ()
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)
  , charge_press(0.0)
  , ip(2.73)
  , bc_hose_to_impulse_line(true)
  , coupling_module_name("sa3")
  , coupling_config_name("sa3")
  , brake_crane_module_name("krm395")
  , brake_crane_config_name("krm395")
  , loco_crane_module_name("kvt254")
  , loco_crane_config_name("kvt254")
  , airdist_module_name("vr483")
  , airdist_config_name("vr483")
  , reg(nullptr)
  , coupling_fwd(nullptr)
  , coupling_bwd(nullptr)
  , oper_rod_fwd(nullptr)
  , oper_rod_bwd(nullptr)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60k::~VL60k()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());
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

    initTractionControl(modules_dir, custom_cfg_dir);

    initOtherEquipment(modules_dir, custom_cfg_dir);

    initTriggers();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::preStep(double t)
{
    preStepCouplings(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::step(double t, double dt)
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

    stepTractionControl(t, dt);

    stepLineContactors(t, dt);

    stepOtherEquipment(t, dt);

    stepSoundSignalsOutput(t, dt);

    stepSignalsOutput();

    debugPrint(t, dt);

    autoStartTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60k)
