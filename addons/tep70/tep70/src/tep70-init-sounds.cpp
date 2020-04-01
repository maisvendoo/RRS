#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::initSounds()
{
    connect(electro_fuel_pump, &ElectricFuelPump::soundPlay, this, &TEP70::soundPlay);
    connect(electro_fuel_pump, &ElectricFuelPump::soundStop, this, &TEP70::soundStop);
    connect(electro_fuel_pump, &ElectricFuelPump::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(electro_fuel_pump, &ElectricFuelPump::soundSetVolume, this, &TEP70::soundSetVolume);
}
