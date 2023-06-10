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

    /// Задать поток в резервуар
    void setFlow(double value);

    /// Задать коэффициент утечек из резервуара в атмосферу
    void setLeakCoeff(double value);

    /// Давление в резервуаре
    double getPressure() const;

private:

    /// Объём резервуара
    double  V;

    /// Поток в резервуар
    double  Q;

    /// Коэффициент утечек из резервуара в атмосферу
    double  k_leak;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);
};

#endif // RESERVOIR_H
