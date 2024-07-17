#ifndef     SL2M_H
#define     SL2M_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SL2M : public Device
{
public:

    SL2M(QObject *parent = Q_NULLPTR);

    ~SL2M();

    void setOmega(double value);

    void setWheelDiameter(double diam);

    float getArrowPos() const;

    float getShaftPos() const;

    /// Состояние звука работы
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал состояния звука работы
    virtual float getSoundSignal(size_t idx = 0) const;

private:

    /// Угловая скорость вращения колесной пары
    double omega;

    /// Передаточное число червячного редуктора
    double ip;

    /// Угловая скорость вращения вала
    double omega_s;

    /// Показатель износа - величина разбега стрелки, км/ч
    double wear_gap;

    /// Максимальна скорость на шкале
    double max_speed;

    /// Сигнал для анимации положения стрелки
    float arrow_pos;

    /// Диаметр бандажа
    double Dk;

    /// Скорость начала работы звука скоростемера
    double speed_begin_sound;
    double omega_begin_sound;

    /// Сигнал положения вала
    float shaft_pos;

    double freq_coeff;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // SL2M_H
