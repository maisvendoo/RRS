#include    "freightcar.h"
#include    "freightcar-signals.h"
#include    "filesystem.h"

#include "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreightCar::FreightCar() : Vehicle ()
{
    analogSignal.resize(SIGNALS_NUM_TOTAL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreightCar::~FreightCar()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getDouble(secName, "BrakepipeLeak", bp_leak);

        cfg.getString(secName, "CouplingModule", coupling_module_name);
        cfg.getString(secName, "CouplingConfig", coupling_config_name);

        cfg.getString(secName, "AirDistModule", air_dist_module);
        cfg.getString(secName, "AirDistConfig", air_dist_config);

        cfg.getString(secName, "ElectroAirDistModule", electro_air_dist_module);
        cfg.getString(secName, "ElectroAirDistConfig", electro_air_dist_config);

        cfg.getDouble(secName, "SupplyReservoirVolume", sr_volume);
        cfg.getDouble(secName, "SupplyReservoirLeak", sr_leak);

        cfg.getString(secName, "BrakepipeAnglecockConfig", anglecock_bp_config);

        cfg.getString(secName, "BrakepipeHoseModule", hose_bp_module);
        cfg.getString(secName, "BrakepipeHoseConfig", hose_bp_config);

        cfg.getString(secName, "BrakeAutomodeModule", automode_module);
        cfg.getString(secName, "BrakeAutomodeConfig", automode_config);

        cfg.getString(secName, "BrakeMechConfig", brake_mech_config);

        cfg.getBool(secName, "isRegistratorOn", is_Registrator_on);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());
    QString custom_cfg_dir(fs.getVehiclesDir().c_str());
    custom_cfg_dir += fs.separator() + config_dir;

    initCouplings(modules_dir, custom_cfg_dir);

    initBrakesEquipment(modules_dir, custom_cfg_dir);

    initEPB(modules_dir, custom_cfg_dir);

    initControl(modules_dir, custom_cfg_dir);

    if (is_Registrator_on)
        initRegistrator(modules_dir, custom_cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::process(const simulator_time_t& t, const double& dt)
{
    disk_end_of_train_fwd.step();
    disk_end_of_train_bwd.step();

    if (needDebugMsg)
        debugPrint(t, dt);

    signalsOutput(t, dt);

    soundsOutput(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::preStep(const double& t)
{
    preStepCouplings(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::step(const double &t, const double &dt)
{
    stepCouplings(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    if (is_Registrator_on)
        stepRegistrator(t, dt);
}

GET_VEHICLE(FreightCar)
