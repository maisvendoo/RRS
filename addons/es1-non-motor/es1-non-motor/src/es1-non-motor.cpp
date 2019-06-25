#include    "es1-non-motor.h"
#include    "physics.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ES1NonMotor::ES1NonMotor() : Vehicle ()
    , brake_pos(MAX_BRAKE_POS)
    , brake_step(0.0)
    , pBC_max(0.5)
    , brake_mech(nullptr)
    , brake_mech_module("carbrakes-mech")
    , brake_mech_config("es1-mech")
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ES1NonMotor::~ES1NonMotor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ES1NonMotor::initialization()
{
    brake_step = pBC_max / MAX_BRAKE_POS;

    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());

    brake_mech = loadBrakeMech(modules_dir + fs.separator() + brake_mech_module);

    if (brake_mech != nullptr)
    {
        brake_mech->read_config(brake_mech_config);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ES1NonMotor::keyProcess()
{
   brake_pos = cut(brake_pos, 0, static_cast<int>(MAX_BRAKE_POS));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ES1NonMotor::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    double pBC_z = brake_pos * brake_step;
    double K1 = 5e-2;
    double p = brake_mech->getBrakeCylinderPressure();
    double Q = K1 * (pBC_z - p);
    brake_mech->setAirFlow(Q);

    for (size_t i = 1; i < Q_a.size(); i++)
    {
        Q_r[i] = brake_mech->getBrakeTorque();
    }

    brake_mech->step(t, dt);

    DebugMsg = QString(" КрМ: %2 ТЦ: %1")
            .arg(p, 4, 'f', 2)
            .arg(brake_pos, 1);
}

GET_VEHICLE(ES1NonMotor)
