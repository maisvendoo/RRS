#include    "test-loco.h"
#include    "physics.h"

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
  , brake_crane_module("krm395")
  , brake_crane(nullptr)
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


    for (size_t i = 0; i < Q_a.size(); i++)
    {
        double torque = traction_level * traction_char(velocity) * wheel_diameter / num_axis / 2.0;
        Q_a[i] = torque;
    }

    if (brake_crane != nullptr)
    {
        brake_crane->setChargePressure(0.5);
        brake_crane->setFeedLinePressure(0.9);
        p0 = brake_crane->getBrakePipeInitPressure();
        brake_crane->setBrakePipePressure(pTM);
        brake_crane->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TestLoco::traction_char(double v)
{
    double max_traction = 600e3;    

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

    if (brake_crane != nullptr)
        brake_crane->read_config(brake_crane_module);
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

        cfg.getString(secName, "BrakeCrane", brake_crane_module);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TestLoco::keyProcess()
{
    double traction_step = 0.1;

    if (keys[97] && !inc_loc)
    {
        traction_level +=  traction_step;
        inc_loc = true;
    }
    else
    {
        inc_loc = false;
    }

    if (keys[100] && !dec_loc)
    {
        traction_level -=  traction_step;
        dec_loc = true;
    }
    else
    {
        dec_loc = false;
    }

    if (keys[111] && !dec_crane_loc)
    {
        dec_crane_loc = true;
        crane_pos--;
        brake_crane->setPosition(crane_pos);
    }
    else
    {
        dec_crane_loc = false;        
    }

    if (keys[112] && !inc_crane_loc)
    {
        inc_crane_loc = true;
        crane_pos++;
        brake_crane->setPosition(crane_pos);
    }
    else
    {
        inc_crane_loc = false;        
    }

    analogSignal[0] = static_cast<float>(traction_level);
    analogSignal[1] = static_cast<float>(brake_crane->getBrakePipeInitPressure());
    analogSignal[2] = static_cast<float>(brake_crane->getEqReservoirPressure());
    analogSignal[3] = crane_pos;
    analogSignal[4] = static_cast<float>(pTM);
}

GET_VEHICLE(TestLoco)
