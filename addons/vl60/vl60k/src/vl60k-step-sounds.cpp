#include    "vl60k.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::stepSoundSignalsOutput(double t, double dt)
{
    (void) t;
    (void) dt;
    // Свисток и тифон
    analogSignal[SOUND_SVISTOK] = horn->getSvistokSound().createSoundSignal();
    analogSignal[SOUND_TIFON] = horn->getTifonSound().createSoundSignal();

    // Дальний ряд тумблеров приборной панели машиниста
//    analogSignal[SOUND_TUMBLER_PROJECTOR2_ON] = proj2_tumbler.getSoundOn().createSoundSignal();
//    analogSignal[SOUND_TUMBLER_PROJECTOR1_ON] = proj1_tumbler.getSoundOn().createSoundSignal();
//    analogSignal[SOUND_TUMBLER_RADIO_ON] = radio_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_CTRL_CIRCUIT_ON] = cu_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANT_BWD_ON] = pant2_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANT_FWD_ON] = pant1_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANTS_ON] = pants_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_GV_RETURN_ON] = gv_return_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_GV_ON] = gv_tumbler.getSoundOn().createSoundSignal();

//    analogSignal[SOUND_TUMBLER_PROJECTOR2_OFF] = proj2_tumbler.getSoundOff().createSoundSignal();
//    analogSignal[SOUND_TUMBLER_PROJECTOR1_OFF] = proj1_tumbler.getSoundOff().createSoundSignal();
//    analogSignal[SOUND_TUMBLER_RADIO_OFF] = radio_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_CTRL_CIRCUIT_OFF] = cu_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANT_BWD_OFF] = pant2_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANT_FWD_OFF] = pant1_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PANTS_OFF] = pants_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_GV_RETURN_OFF] = gv_return_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_GV_OFF] = gv_tumbler.getSoundOff().createSoundSignal();

    // Ближний ряд тумблеров приборной панели машиниста
//    analogSignal[SOUND_TUMBLER_AUTOSAND_ON] = autosand_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN6_ON] = mv_tumblers[MV6].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN5_ON] = mv_tumblers[MV5].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN4_ON] = mv_tumblers[MV4].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN3_ON] = mv_tumblers[MV3].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN2_ON] = mv_tumblers[MV2].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN1_ON] = mv_tumblers[MV1].getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_COMPRESSOR_ON] = mk_tumbler.getSoundOn().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PHASESPLITTER_ON] = fr_tumbler.getSoundOn().createSoundSignal();

//    analogSignal[SOUND_TUMBLER_PROJECTOR2_OFF] = autosand_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN6_OFF] = mv_tumblers[MV6].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN5_OFF] = mv_tumblers[MV5].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN4_OFF] = mv_tumblers[MV4].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN3_OFF] = mv_tumblers[MV3].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN2_OFF] = mv_tumblers[MV2].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_FAN1_OFF] = mv_tumblers[MV1].getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_COMPRESSOR_OFF] = mk_tumbler.getSoundOff().createSoundSignal();
    analogSignal[SOUND_TUMBLER_PHASESPLITTER_OFF] = fr_tumbler.getSoundOff().createSoundSignal();
}
