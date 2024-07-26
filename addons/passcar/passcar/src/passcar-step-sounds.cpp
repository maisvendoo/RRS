#include    <passcar.h>
#include    <passcar-signals.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::stepSoundsSignals(double t, double dt)
{
    (void) t;
    (void) dt;

    double Vkmh = qAbs(velocity) * Physics::kmh;
    analogSignal[SOUND_5_10] = sound_state_t::createSoundSignal((Vkmh > 1.0) && (Vkmh <= 10.0), Vkmh / 5.0);
    analogSignal[SOUND_10_15] = sound_state_t::createSoundSignal((Vkmh > 10.0) && (Vkmh <= 15.0));
    analogSignal[SOUND_15_20] = sound_state_t::createSoundSignal((Vkmh > 15.0) && (Vkmh <= 20.0));
    analogSignal[SOUND_20_30] = sound_state_t::createSoundSignal((Vkmh > 20.0) && (Vkmh <= 30.0));
    analogSignal[SOUND_30_40] = sound_state_t::createSoundSignal((Vkmh > 30.0) && (Vkmh <= 40.0));
    analogSignal[SOUND_40_50] = sound_state_t::createSoundSignal((Vkmh > 40.0) && (Vkmh <= 50.0));
    analogSignal[SOUND_50_60] = sound_state_t::createSoundSignal((Vkmh > 50.0) && (Vkmh <= 60.0));
    analogSignal[SOUND_60_70] = sound_state_t::createSoundSignal((Vkmh > 60.0) && (Vkmh <= 70.0));
    analogSignal[SOUND_70_80] = sound_state_t::createSoundSignal((Vkmh > 70.0) && (Vkmh <= 80.0));
    analogSignal[SOUND_80_90] = sound_state_t::createSoundSignal((Vkmh > 80.0) && (Vkmh <= 90.0));
    analogSignal[SOUND_90_100] = sound_state_t::createSoundSignal((Vkmh > 90.0) && (Vkmh <= 100.0));
    analogSignal[SOUND_100_120] = sound_state_t::createSoundSignal((Vkmh > 100.0) && (Vkmh <= 120.0));
    analogSignal[SOUND_120_140] = sound_state_t::createSoundSignal(Vkmh > 120.0);
}
