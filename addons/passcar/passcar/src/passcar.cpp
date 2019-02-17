#include    "passcar.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCarrige::PassCarrige() : Vehicle ()
  , brake_mech(nullptr)
  , brake_mech_module("carbrakes-mech")
  , brake_mech_config("carbrakes-mech")
  , pz(0.0)
  , inc_loc(false)
  , dec_loc(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCarrige::~PassCarrige()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());
    brake_mech = loadBrakeMech(modules_dir + fs.separator() + brake_mech_module);

    if (brake_mech != nullptr)
    {
        brake_mech->read_config(brake_mech_config);
        brake_mech->setEffFricRadius(wheel_diameter / 2.0);
        brake_mech->setWheelDiameter(wheel_diameter);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::step(double t, double dt)
{
    if (brake_mech != nullptr)
    {
        double p = brake_mech->getBrakeCylinderPressure();
        double K1 = 1e-2;
        pz = Physics::cut(pz, 0.0, 0.4);
        double Q = K1 * (pz - p);

        brake_mech->setAirFlow(Q);
        brake_mech->setVelocity(velocity);
        brake_mech->step(t, dt);
    }

    for (size_t i = 0; i < Q_r.size(); ++i)
    {
        Q_r[i] = brake_mech->getBrakeTorque();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::keyProcess()
{
    double step = 0.1;

    if (keys[KEY_L] && !inc_loc)
    {
        pz +=  step;
        inc_loc = true;
    }
    else
    {
        inc_loc = false;
    }

    if (keys[KEY_K] && !dec_loc)
    {
        pz -=  step;
        dec_loc = true;
    }
    else
    {
        dec_loc = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getString(secName, "BrakeMechModule", brake_mech_module);
        cfg.getString(secName, "BrakeMechCinfig", brake_mech_config);
    }
}

GET_VEHICLE(PassCarrige)
