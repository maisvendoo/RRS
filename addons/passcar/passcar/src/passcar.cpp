#include    "passcar.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCarrige::PassCarrige() : Vehicle ()
  , brake_mech(nullptr)
  , supply_reservoir(nullptr)
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
    supply_reservoir = new Reservoir(0.078);

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
    if ( brake_mech != nullptr )
    {
        double p = brake_mech->getBrakeCylinderPressure();
        double K1 = 1e-2;
        pz = Physics::cut(pz, 0.0, 0.4);
        double Q = K1 * (pz - p);

        brake_mech->setAirFlow(Q);
        brake_mech->setVelocity(velocity);
        brake_mech->step(t, dt);

        for (size_t i = 1; i < Q_r.size(); ++i)
        {
            Q_r[i] = brake_mech->getBrakeTorque();
        }
    }

    if ( supply_reservoir != nullptr )
    {
        supply_reservoir->setAirFlow(0);
        supply_reservoir->step(t, dt);
    }

    DebugMsg = QString("ТМ: %1 ЗР: %2")
            .arg(pTM, 4, 'f', 2)
            .arg(supply_reservoir->getPressure(), 4, 'f', 2);
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
