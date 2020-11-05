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

    void setOmega(double value) { omega = value; }

    void setWheelDiameter(double diam) { Dk = diam; }

    float getArrowPos() const { return arrow_pos; }

    float getShaftPos() const { return shaft_pos; }

private:

    /// Угловая скорость вращения колесной пары
    double omega;

    /// Передаточное число червячного редуктора
    double ip;

    /// Угловая скорость вращения вала
    double omega_s;

    /// Показатель износта - величина разбега стрелки, км/ч
    double wear_gap;

    /// Максимальна скорость на шкале
    double max_speed;

    /// Сигнал для анимации положения стрелки
    float arrow_pos;

    /// Диаметр бандажа
    double Dk;

    double sound_speed;

    /// Сигнал положения вала
    float shaft_pos;

    double freq_coeff;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // SL2M_H
