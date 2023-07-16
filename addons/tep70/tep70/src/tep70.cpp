#include    "tep70.h"

#include    "filesystem.h"

#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70::TEP70() : Vehicle()
  , km(nullptr)
  , battery(nullptr)
  , kontaktor_fuel_pump(nullptr)
  , fuel_tank(nullptr)
  , electro_fuel_pump(nullptr)
  , disel(nullptr)
  , ru8(nullptr)
  , kontaktor_oil_pump(nullptr)
  , oilpump_time_relay(nullptr)
  , starter_time_relay(nullptr)
  , electro_oil_pump(nullptr)
  , starter_generator(nullptr)
  , kontaktor_starter(nullptr)
  , ru10(nullptr)
  , ru6(nullptr)
  , ru42(nullptr)
  , ru7(nullptr)
  , ru15(nullptr)
  , mv6(nullptr)
  , vtn(nullptr)
  , ru4(nullptr)
  , rv4(nullptr)
  , rv9(nullptr)
  , krn(nullptr)
  , voltage_regulator(nullptr)
  , motor_compressor(nullptr)
  , press_reg(nullptr)
  , main_reservoir(nullptr)
  , anglecock_fl_fwd(nullptr)
  , anglecock_fl_bwd(nullptr)
  , hose_fl_fwd(nullptr)
  , hose_fl_bwd(nullptr)
  , ru18(nullptr)
  , ktk1(nullptr)
  , ktk2(nullptr)
  , rv6(nullptr)
  , brake_lock(nullptr)
  , brake_crane(nullptr)
  , loco_crane(nullptr)
  , epk(nullptr)
  , brakepipe(nullptr)
  , air_dist(nullptr)
  , electro_air_dist(nullptr)
  , supply_reservoir(nullptr)
  , anglecock_bp_fwd(nullptr)
  , anglecock_bp_bwd(nullptr)
  , hose_bp_fwd(nullptr)
  , hose_bp_bwd(nullptr)
  , bc_switch_valve(nullptr)
  , bc_splitter(nullptr)
  , anglecock_bc_fwd(nullptr)
  , anglecock_bc_bwd(nullptr)
  , hose_bc_fwd(nullptr)
  , hose_bc_bwd(nullptr)
  , epb_converter(nullptr)
  , epb_control(nullptr)
  , field_gen(nullptr)
  , kvv(nullptr)
  , kvg(nullptr)
  , trac_gen(nullptr)
  , field_reg(nullptr)
  , I_gen(0.0)
  , reg(nullptr)
  , speed_meter(nullptr)
  , button_disel_start(false)
  , button_brake_release(false)
  , button_svistok(false)
  , button_tifon(false)
  , button_RB1(false)
  , Ucc(0.0)
  , Icc(0.0)
  , charge_press(0.5)
  , ip(3.12)
  , ksh1(nullptr)
  , ksh2(nullptr)
  , ru1(nullptr)
  , horn(nullptr)
  , tracForce(0.0)
  , is_svistok(false)
  , is_tifon(false)
  , reversor(nullptr)
  , brake_switcher(nullptr)
  , rp1(nullptr)
  , rp2(nullptr)
  , ksh2_delay(nullptr)
  , ksh1_delay(nullptr)
{
    std::fill(bc_pressure_relay.begin(), bc_pressure_relay.end(), nullptr);
    std::fill(brake_mech.begin(), brake_mech.end(), nullptr);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70::~TEP70()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());

    initCabineControls();

    initControlCircuit();

    initFuelSystem();

    initDisel();

    initOilSystem();

    initPneumoSupply(modules_dir);

    initBrakesControl(modules_dir);

    initBrakesEquipment(modules_dir);

    initEPB(modules_dir);

    initElectroTransmission();

    initOther();

    initSounds();    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    stepCabineControls(t, dt);

    stepControlCircuit(t, dt);

    stepFuelSystem(t, dt);

    stepDisel(t, dt);

    stepOilSystem(t, dt);

    stepPneumoSupply(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    stepElectroTransmission(t, dt);

    stepOther(t, dt);

    stepSignalsOutput(t, dt);

    debugOutput(t, dt);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        double fuel_capacity = 0;
        cfg.getDouble(secName, "FuelCapacity", fuel_capacity);

        double fuel_level = 0;
        cfg.getDouble(secName, "FuelLevel", fuel_level);

        fuel_tank = new FuelTank();
        fuel_tank->setCapacity(fuel_capacity);
        fuel_tank->setFuelLevel(fuel_level);

        cfg.getDouble(secName, "ReductorCoeff", ip);
    }
}

GET_VEHICLE(TEP70)
