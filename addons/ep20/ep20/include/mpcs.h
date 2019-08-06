#ifndef MPCS_H
#define MPCS_H

#include "device.h"
#include "mpcs-data.h"
#include "pant-description.h"
#include "task-pant-state.h"
#include "timer.h"

class MPCS : public Device
{
public:

    MPCS(QObject *parent = Q_NULLPTR);

    ~MPCS();

    /// Инициализация
    void init();

    /// Установка входных сигналов
    void setSignalInputMPCS(const mpcs_input_t &mpcs_input);

    /// Установка пути к файлу
    void setStoragePath(QString path);

    /// Получение выходного сигнала
    mpcs_output_t getSignalOutputMPCS();

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

    /// Выходные значения
    mpcs_input_t mpcs_input;

    /// выходные значения
    mpcs_output_t mpcs_output;

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

    Timer *pantDownWaitingTimer;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

    /// Читать файл последнего рода тока
    void readLastCurrentKind();

    /// Записать в файл последний род тока
    void writeLastCurrentKind();

    /// Конечный автомат (обработка состояний поднятия ТП)
    void taskPantUp(state_vector_t &Y, double t);

    /// Управление клавишами
    void stepKeysControl(double t, double dt);

private slots:

    /// Управление таймером поднятия ТП
    void pantUpTimerHandler();

    void pantDownTimerHandler();
};

#endif // MPCS_H
