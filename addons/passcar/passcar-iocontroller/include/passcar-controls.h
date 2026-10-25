#ifndef     PASSCAR_CONTROLS_H
#define     PASSCAR_CONTROLS_H

//------------------------------------------------------------------------------
// Идентификаторы контролов пассажирского вагона.
// У вагона нет кабин, все контролы общие (CabinesNum = 0)
//------------------------------------------------------------------------------
enum
{
    // Освещение в вагоне
    CTRL_INTERIOR_LIGHT = 100,

    // Красные огни "Хвост поезда"
    CTRL_RED_LAMPS_END_OF_TRAIN_FWD = 101,
    CTRL_RED_LAMPS_END_OF_TRAIN_BWD = 102,

    // Расцепные рычаги автосцепки
    CTRL_OPER_ROD_FWD = 103,
    CTRL_OPER_ROD_BWD = 104,
    CTRL_OPER_ROD_FIX_FWD = 105,   // фиксация в расцепленном положении
    CTRL_OPER_ROD_FIX_BWD = 106,

    // Концевые краны тормозной магистрали
    CTRL_ANGLECOCK_BP_FWD = 107,
    CTRL_ANGLECOCK_BP_BWD = 108,

    // Рукава тормозной магистрали
    CTRL_HOSE_BP_FWD = 109,
    CTRL_HOSE_BP_BWD = 110,

    // Тормозные башмаки
    CTRL_BRAKE_SHOES = 111,
};

#endif // PASSCAR_CONTROLS_H