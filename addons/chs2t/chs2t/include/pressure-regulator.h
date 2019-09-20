#ifndef     PRESSURE_REGULATOR_H
#define     PRESSURE_REGULATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PressureRegulator : public Device
{
public:

    PressureRegulator(double p_min = 0.75,
                      double p_max = 0.9,
                      QObject *parent = Q_NULLPTR);

    ~PressureRegulator();

    void setPressure(double press);

    double getState() const { return state; }

private:

    double p_cur;
    double p_prev;

    double p_min;
    double p_max;

    double state;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PRESSURE_REGULATOR_H
