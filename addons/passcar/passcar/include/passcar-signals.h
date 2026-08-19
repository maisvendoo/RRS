#ifndef     PASSCAR_SIGNALS_H
#define     PASSCAR_SIGNALS_H

enum
{
    WHEEL_1 = 1, ///< Начало массива сигналов для анимации колёсных пар

    IS_BRAKE_SHOWS = 9,  ///< Наличие тормозных колодок

    RED_LAMPS_END_OF_TRAIN_FWD = 10, ///< Включение трёх красных огней на передней торцевой стенке
    RED_LAMPS_END_OF_TRAIN_BWD = 11, ///< Включение трёх красных огней на задней торцевой стенке

    INTERIOR_LIGHT = 12, ///< Освещение в вагоне

    GENERATOR_AXIS = 13, ///< Вращение оси подвагонного генератора

    SOUND_5_10 = 14,
    SOUND_10_15 = 15,
    SOUND_15_20 = 16,
    SOUND_20_30 = 17,
    SOUND_30_40 = 18,
    SOUND_40_50 = 19,
    SOUND_50_60 = 20,
    SOUND_60_70 = 21,
    SOUND_70_80 = 22,
    SOUND_80_90 = 23,
    SOUND_90_100 = 24,
    SOUND_100_120 = 25,
    SOUND_120_140 = 26,

    SIGNALS_NUM_TOTAL
};

#endif // PASSCARSIGNALS_H
