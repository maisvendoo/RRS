#ifndef MPCS_H
#define MPCS_H

#include "device.h"
#include "mpcs-data.h"
#include "pant-description.h"
#include "task-pant-state.h"
#include "timer.h"
#include "mpcs-task-pant-up.h"

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

    ///
    TaskPantUp    *taskPantUp;

    /// Управление кнопками ТП
    Trigger pantControlButton;

    /// Выходные значения
    mpcs_input_t mpcs_input;

    /// выходные значения
    mpcs_output_t mpcs_output;

    /// Управление клавишами
    void stepKeysControl(double t, double dt);

    void stepDiscrete(double t, double dt);

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);
};

#endif // MPCS_H
