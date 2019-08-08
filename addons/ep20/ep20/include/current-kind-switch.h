#ifndef CURRENT_KIND_SWITCH_H
#define CURRENT_KIND_SWITCH_H

#include    "device.h"

class CurrentKindSwitch : public Device
{
public:

    /// Конструктор
    CurrentKindSwitch(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~CurrentKindSwitch();

    /// Установить состояние
    void setState(double state);

    /// Установить напряжение крышевой шины
    void setUkrIn(double Ukr_in);

    /// Получить выходное переменное напряжение
    double getUoutDC() const;

    /// Получить выходное постоянное напряжение
    double getUotAC() const;

private:

    /// Входное состояние
    double state;

    /// Напряжение на крышевой шине
    double Ukr_in;

    /// Выходное переменное напряжение
    double Uout_ac;

    /// Выходное постоянное напряжение
    double Uout_dc;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);
};

#endif // CURRENTKINDSWITCH_H
