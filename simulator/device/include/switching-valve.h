#ifndef     SWITCHING_VALVE_H
#define     SWITCHING_VALVE_H

#include    "brake-device.h"

class DEVICE_EXPORT SwitchingValve : public BrakeDevice
{
public:

    SwitchingValve(double working_volume = 2e-3, QObject *parent = Q_NULLPTR);

    virtual ~SwitchingValve();

    void setInputFlow1(double Q1);

    void setInputFlow2(double Q2);

    void setOutputPressure(double p_out);

    double getPressure1() const;

    double getPressure2() const;

    double getOutputFlow() const;

protected:

    double V_work;

    double K1;

    double K2;

    double A1;

    double p_out;

    double Q1;

    double Q2;

    double Q_out;

    double k;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SWITCHING_VALVE_H
