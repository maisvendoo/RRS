#include    "tep70bs.h"

#include    "filesystem.h"

#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70BS::TEP70BS() : Vehicle()
  , brake_crane_module_name("krm395")
  , brake_crane_config_name("krm395")
  , loco_crane_module_name("kvt254")
  , loco_crane_config_name("kvt254")
  , airdist_module_name("vr292")
  , airdist_config_name("vr292")
  , electro_airdist_module_name("evr305")
  , electro_airdist_config_name("evr305")
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
  , msut(nullptr)
{
    railway_coord = railway_coord0 = 0;
    velocity = 0;

    start_count = 0;
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70BS::~TEP70BS()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70BS::initialization()
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

    initMSUT();

    initAutostart();

    initSounds();

    //reg = new Registrator("../charts/tep70-char", 1.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70BS::step(double t, double dt)
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

    stepMSUTsignals(t, dt);

    stepMSUT(t, dt);

    stepAutostart(t, dt);

    debugOutput(t, dt);

    if (reg == nullptr)
        return;

    QString line = QString("%1 %2 %3")
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(tracForce / 1000.0, 6, 'f', 1)
            .arg(motor[0]->getAncorCurrent(), 6, 'f', 1);
    reg->print(line, t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70BS::loadConfig(QString cfg_path)
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

GET_VEHICLE(TEP70BS)
