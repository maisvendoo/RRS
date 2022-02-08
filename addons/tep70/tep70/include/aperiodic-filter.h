#ifndef     APERIODIC_FILTER_H
#define     APERIODIC_FILTER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AperiodicFilter : public Device
{
public:

    AperiodicFilter(double T = 1.0, QObject *parent = Q_NULLPTR);

    ~AperiodicFilter();

    /// Задать входное воздействие
    void setInputSignal(double u) { this->u = u; }

    /// Задать постоянную времени
    void setTimeConstant(double T) { this->T = T; }

    /// Получить выходной сигнал
    double getOutput() const { return getY(0); }

private:

    /// Постоянная времени фильтра
    double  T;

    /// Входное воздействие
    double  u;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // APERIODIC_FILTER_H
