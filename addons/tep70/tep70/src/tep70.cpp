#include    "tep70.h"

#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70::TEP70() : Vehicle()
  , km(nullptr)
  , battery(nullptr)
  , kontaktor_fuel_pump(nullptr)
  , fuel_tank(nullptr)
  , electro_fuel_pump(nullptr)
  , disel(nullptr)
  , button_disel_start(false)
  , button_brake_release(false)
  , button_svistok(false)
  , button_tifon(false)
  , Ucc(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TEP70::~TEP70()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initialization()
{
    initCabineControls();

    initControlCircuit();

    initFuelSystem();

    initDisel();

    initSounds();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    stepCabineControls(t, dt);

    stepControlCircuit(t, dt);

    stepFuelSystem(t, dt);

    stepDisel(t, dt);

    stepSignalsOutput(t, dt);

    debugOutput(t, dt);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Vehicle";

        double fuel_capacity = 0;
        cfg.getDouble(secName, "FuelCapacity", fuel_capacity);

        double fuel_level = 0;
        cfg.getDouble(secName, "FuelLevel", fuel_level);

        fuel_tank = new FuelTank();
        fuel_tank->setCapacity(fuel_capacity);
        fuel_tank->setFuelLevel(fuel_level);
    }
}

GET_VEHICLE(TEP70)
