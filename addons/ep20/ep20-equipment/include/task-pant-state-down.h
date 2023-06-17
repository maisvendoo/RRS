#ifndef TASK_PANT_STATE_DOWN_H
#define TASK_PANT_STATE_DOWN_H

//------------------------------------------------------------------------------
// Конечный автомат состояний опускания ТП
//------------------------------------------------------------------------------
enum TASK_PANT_DOWN
{
    INITIAL_STATE_IS_DOWN, // Исходное положение ТП
    IS_OFF_MS_HSS, // Проверка состояния ГВ/БВ
    TURN_OFF_PROTECTION_DEVICE, // Выключение ГВ/БВ
    WAITING_TURN_OFF, // Ождание выключения ГВ/БВ
    LOWER_PANT, // Опускание ТП
    WAITING_DOWN_UPON_DOWN, // Ожидание опускания ТП
    FAULT_IS_DOWN // Неисправность
};

#endif // TASKPANTSTATEDOWN_H
