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

    connect(electro_oil_pump, &ElectricOilPump::soundPlay, this, &TEP70::soundPlay);
    connect(electro_oil_pump, &ElectricOilPump::soundStop, this, &TEP70::soundStop);
    connect(electro_oil_pump, &ElectricOilPump::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(electro_oil_pump, &ElectricOilPump::soundSetVolume, this, &TEP70::soundSetVolume);

    connect(starter_generator, &StarterGenerator::soundPlay, this, &TEP70::soundPlay);
    connect(starter_generator, &StarterGenerator::soundStop, this, &TEP70::soundStop);
    connect(starter_generator, &StarterGenerator::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(starter_generator, &StarterGenerator::soundSetVolume, this, &TEP70::soundSetVolume);
}
