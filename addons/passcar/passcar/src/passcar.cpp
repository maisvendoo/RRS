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
  , electroAirDist(nullptr)
  , electro_airdist_module("evr305")
  , electro_airdist_config("evr305")

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

    // Инициализация давления
    brakepipe->setY(0, pTM);

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

    // Тормозная магистраль
    double volumeTM = length * 0.035 * 0.035 * Physics::PI / 4.0;
    brakepipe = new Reservoir(volumeTM);

    // Brake mechanics
    brake_mech = loadBrakeMech(modules_dir + fs.separator() + brake_mech_module);

    // Air supply reservoir
    supply_reservoir = new Reservoir(0.078);

    // Air distributor
    airdist = loadAirDistributor(modules_dir + fs.separator() + airdist_module);

    electroAirDist = loadElectroAirDistributor(modules_dir + fs.separator() + electro_airdist_module);

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

    if (electroAirDist != nullptr)
    {
        electroAirDist->read_config(electro_airdist_config);
    }

    initEPT();
    initSounds();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::step(double t, double dt)
{
    // Подключение потоков из оборудования и межвагонных соединений в ТМ
    double QTM = -1.0 * (airdist->getAuxRate());
    brakepipe->setAirFlow(QTMfwd + QTMbwd + QTM);
    brakepipe->step(t, dt);
    pTMfwd = brakepipe->getPressure();
    pTMbwd = brakepipe->getPressure();

    airdist->setBrakepipePressure(brakepipe->getPressure());
    airdist->setBrakeCylinderPressure(electroAirDist->getPbc_out());
    airdist->setAirSupplyPressure(electroAirDist->getSupplyReservoirPressure());

    auxRate = airdist->getAuxRate();

    airdist->step(t, dt);

    brake_mech->setAirFlow(electroAirDist->getQbc_out());
    brake_mech->setVelocity(velocity);
    brake_mech->step(t, dt);

    for (size_t i = 1; i < Q_r.size(); ++i)
    {
        Q_r[i] = brake_mech->getBrakeTorque();
    }

    supply_reservoir->setAirFlow(electroAirDist->getOutputSupplyReservoirFlow());
    supply_reservoir->step(t, dt);

    electroAirDist->setPbc_in(brake_mech->getBrakeCylinderPressure());
    electroAirDist->setQbc_in(airdist->getBrakeCylinderAirFlow());
    electroAirDist->setSupplyReservoirPressure(supply_reservoir->getPressure());
    electroAirDist->setInputSupplyReservoirFlow(airdist->getAirSupplyFlow());
    electroAirDist->step(t, dt);

    stepSignalsOutput();

    stepEPT(t, dt);

    DebugMsg = QString("t: %1 | %2 км | %3 км/ч ")
            .arg(t, 6, 'f', 1)
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("| FwdQ:%1 | BwdQ:%2 ")
            .arg(QTMfwd, 8, 'f', 5)
            .arg(QTMbwd, 8, 'f', 5);
    DebugMsg += QString("| ТМ:%1 МПа | ЗР:%2 МПа | ТЦ:%3 МПа | Доп.Р.:%4")
            .arg(brakepipe->getPressure(), 5, 'f', 3)
            .arg(supply_reservoir->getPressure(), 5, 'f', 3)
            .arg(brake_mech->getBrakeCylinderPressure(), 5, 'f', 3)
            .arg(auxRate, 8, 'f', 5);
    DebugMsg += QString("| Vbp: %1   ")
            .arg(length * 0.035 * 0.035 * Physics::PI / 4.0, 8, 'f', 5);

    soundStep();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::stepSignalsOutput()
{
    analogSignal[WHEEL_1] = static_cast<float>(dir * wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(dir * wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(dir * wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(dir * wheel_rotation_angle[3] / 2.0 / Physics::PI);

    analogSignal[GEN_MUFTA1] = static_cast<float>(dir * wheel_rotation_angle[2] * 2.96 / 2.0 / Physics::PI);
    analogSignal[GEN_KARDAN] = static_cast<float>(dir * wheel_rotation_angle[2] * 2.96 / 2.0 / Physics::PI);
    analogSignal[GEN_AXIS] = static_cast<float>(dir * wheel_rotation_angle[2] * 2.96 / 2.0 / Physics::PI);
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
        cfg.getString(secName, "BrakeMechConfig", brake_mech_config);

        cfg.getString(secName, "AirDistModule", airdist_module);
        cfg.getString(secName, "AirDistConfig", airdist_config);

        cfg.getString(secName, "ElectroAirDistModule", electro_airdist_module);
        cfg.getString(secName, "ElectroAirDistConfig", electro_airdist_config);
    }
}

GET_VEHICLE(PassCarrige)
