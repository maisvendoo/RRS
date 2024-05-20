#include    "passcar.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCar::PassCar() : Vehicle ()
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
{/*
    // Сцепка/Отцепка спереди
    if (getKeyState(KEY_X))
    {
        if (isShift())
        {
            if (isControl())
            {
                anglecock_bp_fwd->close();
            }
            else
            {
                hose_bp_fwd->disconnect();

                coupling_fwd->uncouple();
            }
        }
        else
        {
            hose_bp_fwd->connect();

            if ( (hose_bp_fwd->isLinked()) || (isControl()) )
                anglecock_bp_fwd->open();

            coupling_fwd->couple();
        }
    }

    // Сцепка/Отцепка сзади
    if (getKeyState(KEY_X))
    {
        if (isShift())
        {
            if (isControl())
            {
                anglecock_bp_bwd->close();
            }
            else
            {
                hose_bp_bwd->disconnect();

                coupling_bwd->uncouple();
            }
        }
        else
        {
            hose_bp_bwd->connect();

            if ( (hose_bp_bwd->isLinked()) || (isControl()) )
                anglecock_bp_bwd->open();

            coupling_bwd->couple();
        }
    }*/
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
