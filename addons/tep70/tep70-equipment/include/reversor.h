#ifndef     REVERSOR_H
#define     REVERSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class   Reversor : public Device
{
public:

    Reversor(QObject *parent = Q_NULLPTR);

    ~Reversor();

    int getSatate() const
    {
        return state;
    }

    void setForwardValveState(bool forward_valve) { this->forward_valve = forward_valve; }

    void setBackwardValveState(bool backward_valve) { this->backward_valve = backward_valve; }

private:

    /// Состояние реверсора (для модели ТЭД)
    int         state;

    /// Максимальная скорость поворота вала реверсора
    double      shaft_omega_max;

    /// Фактическая скорость поворота вала реверсора
    double      omega;

    /// Состояние контакта "Вперед" КМ
    bool        forward_valve;

    /// Сосотояние контакта "Назад" КМ
    bool        backward_valve;

    /// Заданное положение вала реверсора
    double      pos_ref;

    double      K;

    double      eps;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // REVERSOR_H
