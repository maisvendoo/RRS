#ifndef     FREIGHTCAR_CONTROLS_H
#define     FREIGHTCAR_CONTROLS_H

//------------------------------------------------------------------------------
// Идентификаторы контролов грузового вагона.
// У вагона нет кабин, все контролы общие (CabinesNum = 0)
//------------------------------------------------------------------------------
enum
{
    // Сигнальные диски "Хвост грузового поезда"
    CTRL_DISK_END_OF_TRAIN_FWD = 100,
    CTRL_DISK_END_OF_TRAIN_BWD = 101,

    // Расцепные рычаги автосцепки
    CTRL_OPER_ROD_FWD = 102,
    CTRL_OPER_ROD_BWD = 103,
    CTRL_OPER_ROD_FIX_FWD = 104,   // фиксация в расцепленном положении
    CTRL_OPER_ROD_FIX_BWD = 105,

    // Концевые краны тормозной магистрали
    CTRL_ANGLECOCK_BP_FWD = 106,
    CTRL_ANGLECOCK_BP_BWD = 107,

    // Рукава тормозной магистрали
    CTRL_HOSE_BP_FWD = 108,
    CTRL_HOSE_BP_BWD = 109,

    // Тормозные башмаки
    CTRL_BRAKE_SHOES = 110,
};

#endif // FREIGHTCAR_CONTROLS_H