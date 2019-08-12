#ifndef MPCSTASKPANTUP_H
#define MPCSTASKPANTUP_H

#include    "mpcs-task.h"
#include    "task-pant-state-up.h"
#include    "task-pant-state-down.h"
#include    "timer.h"

#include    <QString>

//------------------------------------------------------------------------------
// Нумератор поднятия переднего или заднего ТП
//------------------------------------------------------------------------------
enum
{
    NUM_CONTROLS = 4,
    FORWARD_UP = 0,
    BACKWARD_UP = 1,
    FORWARD_DOWN = 2,
    BACKWARD_DOWN = 3
};



//------------------------------------------------------------------------------
// Класс реализация поднятия/опускания ТП
//------------------------------------------------------------------------------
class TaskPant : public QObject
{
    Q_OBJECT

public:

    /// Конструктор
    TaskPant(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~TaskPant();

    /// Инициализация
    void init();

    /// Шаг моделирования
    void step(state_vector_t &Y, double t,
              double dt,
              const mpcs_input_t &mpcs_input,
              mpcs_output_t &mpcs_output);

    /// Установить контрольные сигналы
    void setControlSignal(size_t id, bool value);

private:

    /// Путь к файлу рода тока
    QString pathStorage;

    /// Выбор кабины машиниста
    int selectedCab;

    /// Последнее значение рода тока
    int last_current_kind;

    /// Состояние поднятия ТП
    TASK_PANT_UP taskPantStateUp;

    /// Состояние опускания ТП
    TASK_PANT_DOWN taskPantStateDown;

    /// Состояние ГВ
    bool isMainSwitch;

    /// Состояние БВ
    bool isHighSpeedSwitch;

    /// Изменеяемое значение рода тока
    int ref_current_kind;

    /// Приоритетный ТП
    size_t pantPriority;

    /// Предыдущий ТП
    int prevPant;

    /// Опускаем ТП
    int downedPant;

    enum
    {
        NUM_PANTS_GROUP = 2
    };

    /// Массив групп ТП
    typedef std::vector<size_t> pant_group_t;
    std::array<pant_group_t, 3> pants;

    /// Таймер ожидания поднятия ТП
    Timer *pantUpWaitingTimer;

    /// Таймер ожидания опускания ТП
    Timer *pantDownWaitingTimer;

    /// Таймер ождания отключения АЗ
    Timer *protectionDeviceWaitingTimer;

    /// Сигналы управления токоприемниками
    std::array<bool, NUM_CONTROLS> ctrl_signals;

    /// Читать файл последнего рода тока
    void readLastCurrentKind();

    /// Записать в файл последний род тока
    void writeLastCurrentKind();

    /// Команда поднятия ТП
    bool isCommandUp();

    /// Команда опускания ТП
    bool isCommandDown();

    /// Обработка состояний поднятия ТП
    void pantUp(const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output);

    /// Обработка состояний опускания ТП
    void pantDown(const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output);

private slots:

    /// Управление таймером поднятия ТП
    void pantUpTimerHandler();

    /// Управление таймером опускания ТП
    void pantDownTimerHandler();

    /// Управление таймером отключения ГВ/БВ
    void protectionDeviceTimerHandler();
};

#endif // MPCSTASKPANTUP_H
