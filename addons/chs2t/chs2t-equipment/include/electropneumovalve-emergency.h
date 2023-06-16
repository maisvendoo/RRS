#ifndef     ELECTROPNEUMOVALVE_EMERGENCY_H
#define     ELECTROPNEUMOVALVE_EMERGENCY_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ElectroPneumoValveEmergency : public Device
{
public:

    ElectroPneumoValveEmergency(double min_pressure = 0.3,
                                double max_pressure = 0.45,
                                QObject *parent = Q_NULLPTR);

    ~ElectroPneumoValveEmergency();

    /// Задать давление экстреннего торможения и отмены экстреннего торможения
    void setPressureRange(double min_pressure, double max_pressure);

    /// Задать давление от питательной магистрали
    void setFLpressure(double value);

    /// Задать давление от тормозной магистрали
    void setBPpressure(double value);

    /// Добавочное давление в нижнюю камеру ДАКО
    double getAdditionalPressure() const;

    /// Разрешение тяги, если тормозная магистраль заряжена
    bool isTractionAllow() const;

    /// Признак экстренного торможения
    bool isEmergency() const;

    void step(double t, double dt);

private:

    Hysteresis *no_emergency;

    double pFL;

    double p_add;

    double p_add_current;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // ELECTROPNEUMOVALVE_EMERGENCY_H
