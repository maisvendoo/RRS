#include    "test-loco.h"

#include    "CfgReader.h"
#include    "filesystem.h"

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TestLoco::TestLoco() : Vehicle()
  , traction_level(0.0)
  , inc_loc(false)
  , dec_loc(false)
  , inc_crane_loc(false)
  , dec_crane_loc(false)
  , crane_pos(1)
  , pz(0.0)
  , inc_brake(false)
  , dec_brake(false)
  , brake_crane_module("krm395")
  , brake_crane_config("krm395")
  , brake_crane(nullptr)  
  , brake_mech(nullptr)
  , brake_mech_module("carbrake-mech")
  , brake_mech_config("tep70bs-mech")
  , charge_press(0.5)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TestLoco::~TestLoco()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::step(double t, double dt)
{
    traction_level = Physics::cut(traction_level, 0.0, 1.0);

    if (brake_mech != nullptr)
    {
        brake_mech->setVelocity(velocity);

        double p = brake_mech->getBrakeCylinderPressure();
        double K1 = 1e-2;
        pz = Physics::cut(pz, 0.0, 0.4);
        double Q = K1 * (pz - p);

        brake_mech->setAirFlow(Q);
        brake_mech->step(t, dt);
    }

    for (size_t i = 1; i < Q_a.size(); i++)
    {
        double torque = traction_level * traction_char(velocity) * wheel_diameter / num_axis / 2.0;
        double brakeTorque = brake_mech->getBrakeTorque();
        Q_a[i] = torque;
        Q_r[i] = brakeTorque;
    }

    if (brake_crane != nullptr)
    {
        brake_crane->setChargePressure(charge_press);
        brake_crane->setFeedLinePressure(0.9);
        p0 = brake_crane->getBrakePipeInitPressure();
        brake_crane->setBrakePipePressure(pTM);
        brake_crane->setPosition(crane_pos);
        brake_crane->step(t, dt);
    }

    emit soundSetPitch("Disel", 1.0f + static_cast<float>(traction_level) / 1.0f);

    DebugMsg = QString("Время: %1 Шаг: %5 Коорд.: %2 Скор.: %3 Тяга: %4 УР: %6 ТМ: %7 ТЦ: %8 КрМ: %9")
            .arg(t, 7, 'f', 1)
            .arg(railway_coord, 10, 'f', 2)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(traction_level, 3, 'f', 1)
            .arg(dt, 8, 'f', 6)
            .arg(brake_crane->getEqReservoirPressure(), 4, 'f', 2)
            .arg(brake_crane->getBrakePipeInitPressure(), 4, 'f', 2)
            .arg(brake_mech->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(brake_crane->getPositionName(crane_pos), 4);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TestLoco::traction_char(double v)
{
    double max_traction = 280e3;

    double vn = 81.0 / Physics::kmh;

    if (abs(v) <= vn)
        return max_traction;
    else
        return max_traction * vn / v;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir(fs.getModulesDir().c_str());
    brake_crane = loadBrakeCrane(modules_dir + fs.separator() + brake_crane_module);
    brake_mech = loadBrakeMech(modules_dir + fs.separator() + brake_mech_module);

    if (brake_crane != nullptr)
    {
        brake_crane->read_config(brake_crane_config);

        brake_mech->read_config(brake_mech_config);
        brake_mech->setEffFricRadius(wheel_diameter / 2.0);
        brake_mech->setWheelDiameter(wheel_diameter);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        cfg.getString(secName, "BrakeCraneModule", brake_crane_module);
        cfg.getString(secName, "BrakeCraneConfig", brake_crane_config);

        cfg.getString(secName, "BrakeMechModule", brake_mech_module);
        cfg.getString(secName, "BrakeMechCinfig", brake_mech_config);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::keyProcess()
{
    incTracTrig.process(keys[KEY_A], traction_level);
    decTracTrig.process(keys[KEY_D], traction_level);

    incBrakeCrane.process(keys[KEY_Rightbracket], crane_pos);
    decBrakeCrane.process(keys[KEY_Leftbracket], crane_pos);


    incChargePress.process(keys[KEY_H], charge_press);
    decChargePress.process(keys[KEY_J], charge_press);

    double step = 0.1;

    if (keys[KEY_L] && !inc_brake)
    {
        pz +=  step;
        inc_brake = true;
    }
    else
    {
        inc_brake = false;
    }

    if (keys[KEY_K] && !dec_brake)
    {
        pz -=  step;
        dec_brake = true;
    }
    else
    {
        dec_brake = false;
    }

    if (keys[KEY_F])
    {
        emit soundSetVolume("Svistok", 100);
    }
    else
    {
        emit soundSetVolume("Svistok", 0);
    }

    if (keys[KEY_G])
    {
        emit soundSetVolume("Tifon", 100);
    }
    else
    {
        emit soundSetVolume("Tifon", 0);
    }

    analogSignal[0] = static_cast<float>(traction_level);
    analogSignal[1] = static_cast<float>(brake_crane->getBrakePipeInitPressure());
    analogSignal[2] = static_cast<float>(brake_crane->getEqReservoirPressure());
    analogSignal[3] = crane_pos;
    analogSignal[4] = static_cast<float>(pTM);
}

GET_VEHICLE(TestLoco)
