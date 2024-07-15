#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSoundSignalsOutput(double t, double dt)
{
    (void) t;
    (void) dt;
    // Свисток и тифон
    analogSignal[SOUND_SVISTOK] = horn->getSoundSignal(TrainHorn::SVISTOK_SOUND);
    analogSignal[SOUND_TIFON] = horn->getSoundSignal(TrainHorn::TIFON_SOUND);

    // Реверсор и контроллер
    analogSignal[SOUND_REVERSOR] = controller->getSoundSignal(ControllerKME_60_044::REVERS_CHANGE_POS_SOUND);
    analogSignal[SOUND_CONTROLLER] = controller->getSoundSignal(ControllerKME_60_044::MAIN_CHANGE_POS_SOUND);

    // Серводвигатель ЭКГ, ручное и автоматическое движение
    analogSignal[SOUND_EKG_ONE] = main_controller->getSoundSignal(EKG_8G::CHANGE_POS_ONE_SOUND);
    analogSignal[SOUND_EKG_AUTO] = main_controller->getSoundSignal(EKG_8G::CHANGE_POS_AUTO_SOUND);
// TODO переделать интерфейс звука на Device::getSoundSignal
    // Токоприёмники
    analogSignal[SOUND_PANT_BWD_UP] = pantographs[1]->getSoundUp().createSoundSignal();
    analogSignal[SOUND_PANT_BWD_DOWN] = pantographs[1]->getSoundDown().createSoundSignal();
    analogSignal[SOUND_PANT_FWD_UP] = pantographs[0]->getSoundUp().createSoundSignal();
    analogSignal[SOUND_PANT_FWD_DOWN] = pantographs[0]->getSoundDown().createSoundSignal();
    // Главный выключатель
    analogSignal[SOUND_GV_ON] = main_switch->getSoundOn().createSoundSignal();
    analogSignal[SOUND_GV_OFF] = main_switch->getSoundOff().createSoundSignal();
    // Трансформатор
    analogSignal[SOUND_TRANSFORMER] = trac_trans->getSoundState().createSoundSignal();
// TODO переделать интерфейс звука на Device::getSoundSignal
    // Мотор-вентиляторы
    analogSignal[SOUND_FAN6] = motor_fans[MV6]->getSoundState().createSoundSignal();
    analogSignal[SOUND_FAN5] = motor_fans[MV5]->getSoundState().createSoundSignal();
    analogSignal[SOUND_FAN4] = motor_fans[MV4]->getSoundState().createSoundSignal();
    analogSignal[SOUND_FAN3] = motor_fans[MV3]->getSoundState().createSoundSignal();
    analogSignal[SOUND_FAN2] = motor_fans[MV2]->getSoundState().createSoundSignal();
    analogSignal[SOUND_FAN1] = motor_fans[MV1]->getSoundState().createSoundSignal();
    // Мотор-компрессор
    analogSignal[SOUND_COMPR] = motor_compressor->getSoundState().createSoundSignal();
    // Фазорасщепитель
    analogSignal[SOUND_PHASESPLITTER] = phase_spliter->getSoundState().createSoundSignal();

    // Дальний ряд тумблеров приборной панели машиниста
//    analogSignal[SOUND_TUMBLER_PROJECTOR2_ON] = proj2_tumbler.getSoundSignal(Trigger::ON_SOUND);
//    analogSignal[SOUND_TUMBLER_PROJECTOR1_ON] = proj1_tumbler.getSoundSignal(Trigger::ON_SOUND);
//    analogSignal[SOUND_TUMBLER_RADIO_ON] = radio_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_CTRL_CIRCUIT_ON] = cu_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_PANT_BWD_ON] = pant2_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_PANT_FWD_ON] = pant1_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_PANTS_ON] = pants_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_GV_RETURN_ON] = gv_return_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_GV_ON] = gv_tumbler.getSoundSignal(Trigger::ON_SOUND);

//    analogSignal[SOUND_TUMBLER_PROJECTOR2_OFF] = proj2_tumbler.getSoundSignal(Trigger::OFF_SOUND);
//    analogSignal[SOUND_TUMBLER_PROJECTOR1_OFF] = proj1_tumbler.getSoundSignal(Trigger::OFF_SOUND);
//    analogSignal[SOUND_TUMBLER_RADIO_OFF] = radio_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_CTRL_CIRCUIT_OFF] = cu_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_PANT_BWD_OFF] = pant2_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_PANT_FWD_OFF] = pant1_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_PANTS_OFF] = pants_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_GV_RETURN_OFF] = gv_return_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_GV_OFF] = gv_tumbler.getSoundSignal(Trigger::OFF_SOUND);

    // Ближний ряд тумблеров приборной панели машиниста
//    analogSignal[SOUND_TUMBLER_AUTOSAND_ON] = autosand_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN6_ON] = mv_tumblers[MV6].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN5_ON] = mv_tumblers[MV5].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN4_ON] = mv_tumblers[MV4].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN3_ON] = mv_tumblers[MV3].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN2_ON] = mv_tumblers[MV2].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_FAN1_ON] = mv_tumblers[MV1].getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_COMPRESSOR_ON] = mk_tumbler.getSoundSignal(Trigger::ON_SOUND);
    analogSignal[SOUND_TUMBLER_PHASESPLITTER_ON] = fr_tumbler.getSoundSignal(Trigger::ON_SOUND);

//    analogSignal[SOUND_TUMBLER_PROJECTOR2_OFF] = autosand_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN6_OFF] = mv_tumblers[MV6].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN5_OFF] = mv_tumblers[MV5].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN4_OFF] = mv_tumblers[MV4].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN3_OFF] = mv_tumblers[MV3].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN2_OFF] = mv_tumblers[MV2].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_FAN1_OFF] = mv_tumblers[MV1].getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_COMPRESSOR_OFF] = mk_tumbler.getSoundSignal(Trigger::OFF_SOUND);
    analogSignal[SOUND_TUMBLER_PHASESPLITTER_OFF] = fr_tumbler.getSoundSignal(Trigger::OFF_SOUND);
}
