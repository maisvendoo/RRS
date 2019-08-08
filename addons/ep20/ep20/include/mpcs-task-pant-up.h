#ifndef MPCSTASKPANTUP_H
#define MPCSTASKPANTUP_H

#include "mpcs-task.h"
#include "task-pant-state.h"
#include "timer.h"
#include "trigger.h"

#include <QString>

//------------------------------------------------------------------------------
// Нумератор поднятия переднего или заднего ТП
//------------------------------------------------------------------------------
enum
{
    NUM_CONTROLS = 2,
    FORWARD_UP = 0,
    BACWARD_UP = 1
};

//------------------------------------------------------------------------------
// Задача поднятия ТП
//------------------------------------------------------------------------------
class TaskPantUp : public QObject
{
    Q_OBJECT

public:

    /// Конструктор
    TaskPantUp(QObject *parent = Q_NULLPTR);

    ~TaskPantUp();

    /// Инициализация
    void init();

    /// Шаг моделирования
    void step(state_vector_t &Y, double t,
              double dt,
              const mpcs_input_t &mpcs_input,
              mpcs_output_t &mpcs_output);

    void setControlSignal(size_t id, bool value);

private:

    /// Путь к файлу рода тока
    QString pathStorage;

    /// Выбор кабины машиниста
    int selectedCab;

    /// Последнее значение рода тока
    int last_current_kind;

    /// Состояние ТП
    TASK_PANT taskPantState;

    /// Изменеяемое значение рода тока
    int ref_current_kind;

    /// Приоритетный ТП
    size_t pantPriority;

    /// Предыдущий ТП
    int prevPant;

    enum
    {
        NUM_PANTS_GROUP = 2
    };

    /// Массив групп ТП
    typedef std::vector<size_t> pant_group_t;
    std::array<pant_group_t, 3> pants;

    /// Управление кнопками ТП
    Trigger pantControlButton;

    /// Таймер ожидания поднятия ТП
    Timer *pantUpWaitingTimer;

    /// Таймер ожидания опускания ТП
    Timer *pantDownWaitingTimer;

    /// Сигналы управления токоприемниками
    std::array<bool, NUM_CONTROLS> ctrl_signals;

    /// Читать файл последнего рода тока
    void readLastCurrentKind();

    /// Записать в файл последний род тока
    void writeLastCurrentKind();

    bool isCommandUp();

private slots:

    /// Управление таймером поднятия ТП
    void pantUpTimerHandler();

    /// Управление таймером опускания ТП
    void pantDownTimerHandler();
};

#endif // MPCSTASKPANTUP_H
