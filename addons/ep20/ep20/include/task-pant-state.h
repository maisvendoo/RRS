#ifndef TASKPANTSTATE_H
#define TASKPANTSTATE_H

//------------------------------------------------------------------------------
// Конечный автомат состояний поднятия ТП
//------------------------------------------------------------------------------
enum TASK_PANT
{
    INITIAL_STATE, // Исходное положение ТП
    UP_PRIORETY_PANT, // Подъем приоритетного ТП
    WAITING_UP, // Ожидание подъема ТП
    CONTROL_CURRENT_KIND, // Контроль РТ
    CHANGE_KURRENT_KIND, // Смена РТ
    LOWER_FIRST_PANT, // Опускание первого поднятого ТП
    UP_RESERVE_PANT, // Подъем резервного ТП
    WAITING_DOWN, // Ожидание опускание ТП
    READY, // Готовность (Рабочее состояние)
    FAULT // Неисправность
};

#endif // TASKPANTSTATE_H
