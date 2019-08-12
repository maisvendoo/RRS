#ifndef TASKPANTSTATE_H
#define TASKPANTSTATE_H

//------------------------------------------------------------------------------
// Конечный автомат состояний поднятия ТП
//------------------------------------------------------------------------------
enum TASK_PANT_UP
{
    INITIAL_STATE_IS_UP, // Исходное положение ТП
    UP_PRIORETY_PANT, // Подъем приоритетного ТП
    WAITING_UP, // Ожидание подъема ТП
    CONTROL_CURRENT_KIND, // Контроль РТ
    CHANGE_KURRENT_KIND, // Смена РТ
    LOWER_FIRST_PANT, // Опускание первого поднятого ТП
    WAITING_DOWN_UPON_UP, // Ожидание опускание ТП
    READY, // Готовность (Рабочее состояние)
    FAULT_IS_UP // Неисправность
};

#endif // TASKPANTSTATE_H
