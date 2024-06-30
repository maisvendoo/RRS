#ifndef     BATTERY_H
#define     BATTERY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Battery : public Device
{
public:

    Battery(QObject *parent = Q_NULLPTR);

    ~Battery();

    /// Задать напряжение заряда
    void setChargeVoltage(double U);

    /// Задать ток, потребляемый нагрузкой
    void setLoadCurrent(double I);

    /// Задать ток на якоре стартера-генератора дизеля
    void setStarterCurrent(double I);

    /// Ток заряда/разряда
    double getChargeCurrent() const;

    /// Напряжение на выходе
    double getVoltage() const;

private:

    /// Внутреннее сопротивление батареи
    double  r;

    /// Добавочное сопротивление в цепи заряда
    double  Rd;

    /// Ток, потребляемый нагрузкой
    double  In;

    /// Ток, потребляемый стартером дизеля
    double  Is;

    /// Ток заряда/разряда
    double  Ib;

    /// Максимальная ЭДС
    double  Emax;

    /// Минимальная ЭДС
    double  Emin;

    /// Емкость, А*ч
    double  C;

    /// Напряжение заряда
    double  U_gen;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // BATTERY_H
