#ifndef     PRESSURE_REGULATOR_H
#define     PRESSURE_REGULATOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT PressureRegulator : public Device
{
public:

    PressureRegulator(double min_pressure = 0.75,
                      double max_pressure = 0.9,
                      QObject *parent = Q_NULLPTR);

    ~PressureRegulator();

    void setPressureRange(double min_pressure, double max_pressure);

    void setFLpressure(double fl_pressure);

    bool getState() const;

private:

    Hysteresis *hysteresis;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PRESSURE_REGULATOR_H
