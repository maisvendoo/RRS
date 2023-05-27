#ifndef     SWITCHING_VALVE_H
#define     SWITCHING_VALVE_H

#include    "brake-device.h"

class DEVICE_EXPORT SwitchingValve : public BrakeDevice
{
public:

    SwitchingValve(double working_volume = 2e-3, QObject *parent = Q_NULLPTR);

    virtual ~SwitchingValve();

    /// Задать поток из первой входящей магистрали
    void setInputFlow1(double value);

    /// Давление в первой рабочей камере переключательного клапана
    double getPressure1() const;

    /// Задать поток из второй входящей магистрали
    void setInputFlow2(double value);

    /// Давление во второй рабочей камере переключательного клапана
    double getPressure2() const;

    /// Задать давление от исходящей магистрали
    void setOutputPressure(double value);

    /// Поток в исходящую магистраль
    double getOutputFlow() const;

protected:

    /// Объём рабочих камер
    double V0;

    double pOUT;

    double QIN1;
    double QIN2;
    double QOUT;

    double K1;

    double K2;

    double A1;

    double k;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SWITCHING_VALVE_H
