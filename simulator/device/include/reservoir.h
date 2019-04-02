#ifndef     RESERVOIR_H
#define     RESERVOIR_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Reservoir : public BrakeDevice
{
public:

    Reservoir(double volume, QObject *parent = Q_NULLPTR);

    ~Reservoir();

    void setAirFlow(double Q);

    double getPressure() const;    

private:

    double  V;
    double  Q;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
};

#endif // RESERVOIR_H
