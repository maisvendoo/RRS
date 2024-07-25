#ifndef     FREIGHTCARSIGNALS_H
#define     FREIGHTCARSIGNALS_H

enum
{
    WHEEL_1 = 1, ///< Начало массива сигналов для анимации колёсных пар

    // Зарезервировал 8 + 1 место под колесные пары вагонов, звуки
    // начинаю с 10
    SOUND_4_10 = 10,
    SOUND_10_20 = 11,
    SOUND_20_30 = 12,
    SOUND_30_40 = 13,
    SOUND_40_50 = 14,
    SOUND_50_60 = 15,
    SOUND_60_70 = 16,
    SOUND_70_80 = 17
};

#endif // FREIGHTCARSIGNALS_H
