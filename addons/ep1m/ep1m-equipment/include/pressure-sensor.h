#ifndef     PRESSURE_SENSOR_H
#define     PRESSURE_SENSOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PressureSensor : public Device
{
public:

    PressureSensor(QObject *parent = Q_NULLPTR);

    ~PressureSensor();

    void setInputPressure(double p_in)
    {
        this->p_in = p_in;
    }

    bool getOpenContactState() const
    {
        return getY(0) >= p_ref;
    }

    bool getClosedContactState() const
    {
        return getY(0) <= p_ref;
    }

private:

    /// Давление на входе датчика
    double p_in;

    /// Давление уставки
    double p_ref;

    /// Эквивалентный объем полостей
    double Vk;

    /// Коэффициент расхода
    double K1;

    void preStep(state_vector_t &Y, double t) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    void load_config(CfgReader &cfg) override;
};

#endif // PRESSURE_SENSOR_H
