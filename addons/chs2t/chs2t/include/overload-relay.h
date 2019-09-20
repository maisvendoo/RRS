#ifndef     OVERLOADRELAY_H
#define     OVERLOADRELAY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OverloadRelay : public Device
{
public:

    OverloadRelay(QObject *parent = Q_NULLPTR);

    ~OverloadRelay();

    void setCurrent(double value) { current = value; }

    double getState() const { return state; }

private:

    /// Измеряемый ток
    double current;

    /// Уставка срабатывания реле
    double trig_current;

    /// Состояние реле
    double state;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // OVERLOADRELAY_H
