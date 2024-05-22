#include    "passcar.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCar::PassCar() : Vehicle ()
  , coupling_fwd(nullptr)
  , coupling_bwd(nullptr)
  , coupling_module_name("sa3")
  , coupling_config_name("sa3")
  , oper_rod_fwd(nullptr)
  , oper_rod_bwd(nullptr)
  , brakepipe(nullptr)
  , bp_leak(0.0)
  , air_dist(nullptr)
  , air_dist_module("vr242")
  , air_dist_config("vr242")
  , electro_air_dist(nullptr)
  , electro_air_dist_module("")
  , electro_air_dist_config("")
  , supply_reservoir(nullptr)
  , sr_volume(0.078)
  , sr_leak(0.0)
  , anglecock_bp_fwd(nullptr)
  , anglecock_bp_bwd(nullptr)
  , anglecock_bp_config("pneumo-anglecock-BP")
  , hose_bp_fwd(nullptr)
  , hose_bp_bwd(nullptr)
  , hose_bp_module("hose369a")
  , hose_bp_config("pneumo-hose-BP369a-passcar")
  , brake_mech(nullptr)
  , brake_mech_config("carbrakes-mech-composite")
  , ip(2.96)
  , is_Registrator_on(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCar::~PassCar()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());

    initCouplings( modules_dir);

    initBrakesEquipment(modules_dir);

    initEPB(modules_dir);

    initSounds();

    if (is_Registrator_on)
        initRegistrator();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::step(double t, double dt)
{
    stepCouplings(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    stepSignalsOutput();

    stepDebugMsg(t, dt);

    soundStep();

    if (is_Registrator_on)
        stepRegistrator(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::keyProcess()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::loadConfig(QString cfg_path)
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

        cfg.getString(secName, "BrakeMechConfig", brake_mech_config);

        cfg.getDouble(secName, "GenReductorCoeff", ip);

        cfg.getBool(secName, "isRegistratorOn", is_Registrator_on);
    }
}

GET_VEHICLE(PassCar)
