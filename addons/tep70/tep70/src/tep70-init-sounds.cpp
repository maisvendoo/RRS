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

    azv_common_control.setOnSoundName("AZV_On");
    azv_common_control.setOffSoundName("AZV_Off");
    connect(&azv_common_control, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_fuel_pump.setOnSoundName("AZV_On");
    azv_fuel_pump.setOffSoundName("AZV_Off");
    connect(&azv_fuel_pump, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_motor_compressor.setOnSoundName("AZV_On");
    azv_motor_compressor.setOffSoundName("AZV_Off");
    connect(&azv_motor_compressor, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_edt_power.setOnSoundName("AZV_On");
    azv_edt_power.setOffSoundName("AZV_Off");
    connect(&azv_edt_power, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_upr_tepl.setOnSoundName("AZV_On");
    azv_upr_tepl.setOffSoundName("AZV_Off");
    connect(&azv_upr_tepl, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_ept_on.setOnSoundName("AZV_On");
    azv_ept_on.setOffSoundName("AZV_Off");
    connect(&azv_ept_on, &Trigger::soundPlay, this, &TEP70::soundPlay);

    azv_edt_on.setOnSoundName("AZV_On");
    azv_edt_on.setOffSoundName("AZV_Off");
    connect(&azv_edt_on, &Trigger::soundPlay, this, &TEP70::soundPlay);

    tumbler_voltage.setOnSoundName("Tumbler_On");
    tumbler_voltage.setOffSoundName("Tumbler_Off");
    connect(&tumbler_voltage, &Trigger::soundPlay, this, &TEP70::soundPlay);

    tumbler_disel_stop.setOnSoundName("Tumbler_On");
    tumbler_disel_stop.setOffSoundName("Tumbler_Off");
    connect(&tumbler_disel_stop, &Trigger::soundPlay, this, &TEP70::soundPlay);

    tumbler_field_weak1.setSoundName("Switcher");
    connect(&tumbler_field_weak1, &TEP70Switcher::soundPlay, this, &TEP70::soundPlay);

    tumbler_field_weak2.setSoundName("Switcher");
    connect(&tumbler_field_weak2, &TEP70Switcher::soundPlay, this, &TEP70::soundPlay);

    tumbler_water_zaluzi.setSoundName("Switcher");
    connect(&tumbler_water_zaluzi, &TEP70Switcher::soundPlay, this, &TEP70::soundPlay);

    tumbler_oil_zaluzi.setSoundName("Switcher");
    connect(&tumbler_oil_zaluzi, &TEP70Switcher::soundPlay, this, &TEP70::soundPlay);

    ru8->setSoundName("Relay");
    connect(ru8, &Relay::soundPlay, this, &TEP70::soundPlay);

    kontaktor_fuel_pump->setSoundName("Relay");
    connect(kontaktor_fuel_pump, &Relay::soundPlay, this, &TEP70::soundPlay);

    kontaktor_oil_pump->setSoundName("Relay");
    connect(kontaktor_oil_pump, &Relay::soundPlay, this, &TEP70::soundPlay);

    oilpump_time_relay->setSoundName("Relay");
    connect(oilpump_time_relay, &Relay::soundPlay, this, &TEP70::soundPlay);

    starter_time_relay->setSoundName("Relay");
    connect(starter_time_relay, &Relay::soundPlay, this, &TEP70::soundPlay);

    kontaktor_starter->setSoundName("Relay");
    connect(kontaktor_starter, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru10->setSoundName("Relay");
    connect(ru10, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru6->setSoundName("Relay");
    connect(ru6, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru42->setSoundName("Relay");
    connect(ru42, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru15->setSoundName("Relay");
    connect(ru15, &Relay::soundPlay, this, &TEP70::soundPlay);

    vtn->setSoundName("Relay");
    connect(vtn, &Relay::soundPlay, this, &TEP70::soundPlay);

    mv6->setSoundName("Relay");
    connect(mv6, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru4->setSoundName("Relay");
    connect(ru4, &Relay::soundPlay, this, &TEP70::soundPlay);

    rv9->setSoundName("Relay");
    connect(rv9, &Relay::soundPlay, this, &TEP70::soundPlay);

    krn->setSoundName("Relay");
    connect(krn, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru18->setSoundName("Relay");
    connect(ru18, &Relay::soundPlay, this, &TEP70::soundPlay);

    ktk1->setSoundName("Relay");
    connect(ktk1, &Relay::soundPlay, this, &TEP70::soundPlay);

    ktk2->setSoundName("Relay");
    connect(ktk2, &Relay::soundPlay, this, &TEP70::soundPlay);

    rv6->setSoundName("Relay");
    connect(rv6, &Relay::soundPlay, this, &TEP70::soundPlay);

    kvv->setSoundName("Relay");
    connect(kvv, &Relay::soundPlay, this, &TEP70::soundPlay);

    kvg->setSoundName("Relay");
    connect(kvg, &Relay::soundPlay, this, &TEP70::soundPlay);

    ksh1->setSoundName("Relay");
    connect(ksh1, &Relay::soundPlay, this, &TEP70::soundPlay);

    ksh2->setSoundName("Relay");
    connect(ksh2, &Relay::soundPlay, this, &TEP70::soundPlay);

    ru1->setSoundName("Relay");
    connect(ru1, &Relay::soundPlay, this, &TEP70::soundPlay);

    //connect(horn, &TEP70Horn::soundPlay, this, &TEP70::soundPlay);
    //connect(horn, &TEP70Horn::soundStop, this, &TEP70::soundStop);

    connect(km, &ControllerKM2202::soundPlay, this, &TEP70::soundPlay);
}
