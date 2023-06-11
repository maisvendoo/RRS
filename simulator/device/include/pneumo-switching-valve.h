#ifndef     SWITCHING_VALVE_H
#define     SWITCHING_VALVE_H

#include    "brake-device.h"

class DEVICE_EXPORT SwitchingValve : public BrakeDevice
{
public:

    SwitchingValve(double working_volume_1 = 1e-3,
                   double working_volume_2 = 1e-3,
                   QObject *parent = Q_NULLPTR);

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

    /// Объём рабочей камеры 1
    double V1;
    /// Объём рабочей камеры 2
    double V2;

    double pOUT;

    double QIN1;
    double QIN2;
    double QOUT;

    double K1;

    double A1;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SWITCHING_VALVE_H
