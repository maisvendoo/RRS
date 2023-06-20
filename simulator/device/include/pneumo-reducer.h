#ifndef     PNEUMO_REGUCER_H
#define     PNEUMO_REGUCER_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoReducer : public BrakeDevice
{
public:

    PneumoReducer(double ref_pressure = 0.5, QObject *parent = Q_NULLPTR);

    ~PneumoReducer();

    /// Задать уставку давления на выходе пневморедуктора
    void setRefPressure(double value);

    /// Задать давление на входе редуктора
    void setInputPressure(double value);

    /// Поток в редуктор
    double getInputFlow() const;

    /// Задать поток из выхода пневморедуктора
    void setOutFlow(double value);

    /// Давление на выходе пневморедуктора
    double getOutPressure() const;

private:

    /// Объем рабочей полости на выходе
    double V0;

    /// Уставка давления на выходе редуктора
    double pREF;

    /// Давление на входе редуктора
    double pIN;

    /// Расход из рабочей полости редуктора
    double QOUT;

    /// Расход в рабочую полость
    double QIN;

    /// Коэффициент к условному клапану наполнения рабочей полости
    double kv;

    /// Коэффициент к потоку наполнения рабочей полости
    double Kflow;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PNEUMO_REGUCER_H
