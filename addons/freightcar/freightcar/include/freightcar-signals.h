#ifndef     FREIGHTCARSIGNALS_H
#define     FREIGHTCARSIGNALS_H

enum
{
    WHEEL_1 = 1, ///< Начало массива сигналов для анимации колёсных пар
    // Зарезервированы сигналы 1-8 под колесные пары вагона

    IS_BRAKE_SHOWS = 9,

    DISK_END_OF_TRAIN_FWD = 10, ///< Наличие сигнального диска на переднем брусе
    DISK_END_OF_TRAIN_BWD = 11, ///< Наличие сигнального диска на заднем брусе

    // Звуки перестука в движении
    SOUND_4_10 = 12,
    SOUND_10_20 = 13,
    SOUND_20_30 = 14,
    SOUND_30_40 = 15,
    SOUND_40_50 = 16,
    SOUND_50_60 = 17,
    SOUND_60_70 = 18,
    SOUND_70_80 = 19,

    SIGNALS_NUM_TOTAL
};

#endif // FREIGHTCARSIGNALS_H
