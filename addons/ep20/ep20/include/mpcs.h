#ifndef MPCS_H
#define MPCS_H

#include    "device.h"
#include    "mpcs-data.h"
#include    "pant-description.h"
#include    "timer.h"
#include    "mpcs-task-pant.h"
#include    "auxiliary-converter.h"

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

    float getKeyPosition();

private:

    /// Путь к файлу рода тока
    QString pathStorage;

    /// Состояния ТП
    TaskPant    *taskPant;

    /// Выходные значения
    mpcs_input_t mpcs_input;

    /// Выходные значения
    mpcs_output_t mpcs_output;

    /// Кнопка ГВ/БВ
    Trigger ms_fs_on;

    AuxiliaryConverter  *auxConv;

    Timer               mkStartTimer;

    Timer               blinkButtonsTimer;

    std::array<double, 2> mk_start;

    size_t                mk_count;

    double p_prev;

    double p_min;

    double p_max;

    // Позиция ключа МПСУ
    int keyPosition;

    // Позиция выключателя цепей управления
    int controlSwitch;

    /// Управление клавишами
    void stepKeysControl(double t, double dt);

    void stepDiscrete(double t, double dt);

    /// Контроль ГВ
    void stepMainSwitchControl(double t, double dt);

    /// Контроль БВ
    void stepFastSwitchControl(double t, double dt);

    void stepToggleSwitchMK(double t, double dt);

    void PressureReg();

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    void postStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

    /// Включение ламп на сенсорных кнопках
    void buttonsOn();

    /// Выключение ламп на сенсорных кнопках
    void buttonsOff();

private slots:

    void slotMKStart();

    void slotBlinkButtons();
};

#endif // MPCS_H
