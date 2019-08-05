#include    "passcar.h"
#include    "filesystem.h"
#include    "passcar-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PassCarrige::PassCarrige() : Vehicle ()
  , brake_mech(nullptr)
  , supply_reservoir(nullptr)
  , brake_mech_module("carbrakes-mech")
  , brake_mech_config("carbrakes-mech")
  , airdist(nullptr)
  , airdist_module("vr242")
  , airdist_config("vr242")

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
void PassCarrige::initBrakeDevices(double p0, double pTM, double pFL)
{
    Q_UNUSED(p0)

    if (supply_reservoir != nullptr)
        supply_reservoir->setY(0, pTM);

    if (airdist != nullptr)
        airdist->init(pTM, pFL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());

    // Brake mechanics
    brake_mech = loadBrakeMech(modules_dir + fs.separator() + brake_mech_module);

    // Air supply reservoir
    supply_reservoir = new Reservoir(0.078);

    // Air distributor
    airdist = loadAirDistributor(modules_dir + fs.separator() + airdist_module);

    if (brake_mech != nullptr)
    {
        brake_mech->read_config(brake_mech_config);
        brake_mech->setEffFricRadius(wheel_diameter / 2.0);
        brake_mech->setWheelDiameter(wheel_diameter);
    }

    if (airdist != nullptr)
    {
        airdist->read_config(airdist_config);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::step(double t, double dt)
{
    if ( brake_mech != nullptr )
    {
        brake_mech->setAirFlow(airdist->getBrakeCylinderAirFlow());
        brake_mech->setVelocity(velocity);
        brake_mech->step(t, dt);

        for (size_t i = 1; i < Q_r.size(); ++i)
        {
            Q_r[i] = brake_mech->getBrakeTorque();
        }
    }

    if ( supply_reservoir != nullptr )
    {
        supply_reservoir->setAirFlow(airdist->getAirSupplyFlow());
        supply_reservoir->step(t, dt);
    }

    if ( airdist != nullptr)
    {
        airdist->setBrakepipePressure(pTM);
        airdist->setBrakeCylinderPressure(brake_mech->getBrakeCylinderPressure());
        airdist->setAirSupplyPressure(supply_reservoir->getPressure());

        auxRate = airdist->getAuxRate();

        airdist->step(t, dt);
    }

    stepSignalsOutput();

    DebugMsg = QString("Время: %3 ТМ: %1 ЗР: %2")
            .arg(pTM, 4, 'f', 2)
            .arg(supply_reservoir->getPressure(), 4, 'f', 2)
            .arg(t, 10, 'f', 1);

    DebugMsg += airdist->getDebugMsg();

    DebugMsg += QString(" Тепм. ДР.: %1")
            .arg(auxRate, 9, 'f', 4);   
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::stepSignalsOutput()
{
    analogSignal[WHEEL_1] = static_cast<float>(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(wheel_rotation_angle[3] / 2.0 / Physics::PI);

    analogSignal[GEN_MUFTA1] = static_cast<float>(wheel_rotation_angle[2] * 2.96 / 2.0 / Physics::PI);
    analogSignal[GEN_KARDAN] = static_cast<float>(wheel_rotation_angle[2] * 2.96 / 2.0 / Physics::PI);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::keyProcess()
{

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

        cfg.getString(secName, "AirDistModule", airdist_module);
        cfg.getString(secName, "AirDistConfig", airdist_config);
    }
}

GET_VEHICLE(PassCarrige)
