#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::horn_play(QString name)
{
    if ( name == QString("Svistok") )
        ss_svistok.play = true;
    if ( name == QString("Tifon") )
        ss_tifon.play = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::horn_stop(QString name)
{
    if ( name == QString("Svistok") )
        ss_svistok.play = false;
    if ( name == QString("Tifon") )
        ss_tifon.play = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::horn_volume(QString name, int volume)
{
    if ( name == QString("Svistok") )
        ss_svistok.volume = static_cast<float>(volume) / 100.0f;
    if ( name == QString("Tifon") )
        ss_tifon.volume = static_cast<float>(volume) / 100.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSoundSignalsOutput(double t, double dt)
{
    (void) t;
    (void) dt;
    analogSignal[SOUND_SVISTOK] = ss_svistok.createSoundSignal();
    analogSignal[SOUND_TIFON] = ss_tifon.createSoundSignal();
}
