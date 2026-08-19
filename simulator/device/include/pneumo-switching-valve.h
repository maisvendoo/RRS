#ifndef     SWITCHING_VALVE_H
#define     SWITCHING_VALVE_H

#include    "brake-device.h"

class DEVICE_EXPORT SwitchingValve : public BrakeDevice
{
public:

    SwitchingValve(double working_volume_1 = 1e-3,
                   double working_volume_2 = 1e-3,
                   QObject *parent = nullptr);

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

    /// Игнорировать условный объём камеры со стороны, куда открыт клапан,
    /// напрямую передавая поток и давление в смежное оборудование без задержек
    bool ignore_volume1 = false;
    bool ignore_volume2 = false;

    /// Объём рабочей камеры 1
    double V1 = 1.0e-3;
    /// Объём рабочей камеры 2
    double V2 = 1.0e-3;

    double pOUT = 0.0;

    double QIN1 = 0.0;
    double QIN2 = 0.0;
    double QOUT = 0.0;

    /// Коэффициент перетока к выходу из рабочей камеры со стороны, куда открыт клапан
    double K1 = 5.0e-2;

    /// Коэффициент к скорости переключенияs
    double A1 = 100.0;

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SWITCHING_VALVE_H
