#ifndef     BATTERY_H
#define     BATTERY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Battery : public Device
{
public:

    Battery(QObject* parent = nullptr);

    ~Battery() = default;

    /// Задать напряжение заряда
    void setChargeVoltage(double U) noexcept;

    /// Задать ток, потребляемый нагрузкой
    void setLoadCurrent(double I) noexcept;

    /// Задать ток на якоре стартера-генератора дизеля
    void setStarterCurrent(double I) noexcept;

    /// Ток заряда/разряда
    double getChargeCurrent() const noexcept;

    /// Напряжение на выходе
    double getVoltage() const;

private:

    /// Внутреннее сопротивление батареи
    double  r = 1.0;

    /// Добавочное сопротивление в цепи заряда
    double  Rd = 1.0;

    /// Ток, потребляемый нагрузкой
    double  In = 0.0;

    /// Ток, потребляемый стартером дизеля
    double  Is = 0.0;

    /// Ток заряда/разряда
    double  Ib = 0.0;

    /// Максимальная ЭДС
    double  Emax = 96.0;

    /// Минимальная ЭДС
    double  Emin = 84.0;

    /// Емкость, А*ч
    double  C = 450.0;

    /// Напряжение заряда
    double  U_gen = 0.0;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // BATTERY_H
