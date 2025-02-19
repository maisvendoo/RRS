#ifndef     VIGILANSECHECKSTATEENUM_H
#define     VIGILANSECHECKSTATEENUM_H

/*!
 * \enum SignalVigilanseCheckState
 * \brief Перечислитель состояний индикации сигнала "проверка бдительности"
 */
enum SignalVigilanseCheckState
{
    NoneIndication,         ///< Нет индикации
    FlashingRedTriangle,    ///< Мигающий красный треугольник
    LitRedTriangle,         ///< Зажжённый красный треугольник
    LitYellowTriangle       ///< Зажжённый жёлтый треугольник
};

#endif // VIGILANSECHECKSTATEENUM_H
