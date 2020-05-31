#ifndef     DRIVER_CONTROLLER_H
#define     DRIVER_CONTROLLER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DriverController : public Device
{
public:

    DriverController(QObject *parent = Q_NULLPTR);

    ~DriverController();

    double getPosition() const { return pos; }

private:

    double pos;

    double pos_rate;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &, state_vector_t &, double);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);
};

#endif // DRIVER_CONTROLLER_H
