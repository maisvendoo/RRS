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

    connect(disel, &Disel::soundPlay, this, &TEP70::soundPlay);
    connect(disel, &Disel::soundStop, this, &TEP70::soundStop);
    connect(disel, &Disel::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(disel, &Disel::soundSetVolume, this, &TEP70::soundSetVolume);

    connect(motor_compressor, &DCMotorCompressor::soundPlay, this, &TEP70::soundPlay);
    connect(motor_compressor, &DCMotorCompressor::soundStop, this, &TEP70::soundStop);
    connect(motor_compressor, &DCMotorCompressor::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(motor_compressor, &DCMotorCompressor::soundSetVolume, this, &TEP70::soundSetVolume);

    connect(ubt367m, &BrakeLock::soundPlay, this, &TEP70::soundPlay);
    connect(ubt367m, &BrakeLock::soundStop, this, &TEP70::soundStop);
    connect(ubt367m, &BrakeLock::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(ubt367m, &BrakeLock::soundSetVolume, this, &TEP70::soundSetVolume);

    connect(krm, &BrakeCrane::soundPlay, this, &TEP70::soundPlay);
    connect(krm, &BrakeCrane::soundStop, this, &TEP70::soundStop);
    connect(krm, &BrakeCrane::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(krm, &BrakeCrane::soundSetVolume, this, &TEP70::soundSetVolume);

    connect(kvt, &LocoCrane::soundPlay, this, &TEP70::soundPlay);
    connect(kvt, &LocoCrane::soundStop, this, &TEP70::soundStop);
    connect(kvt, &LocoCrane::soundSetPitch, this, &TEP70::soundSetPitch);
    connect(kvt, &LocoCrane::soundSetVolume, this, &TEP70::soundSetVolume);
}
