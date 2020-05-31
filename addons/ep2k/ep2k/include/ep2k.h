#ifndef     EP2K_H
#define     EP2K_H

#include    "vehicle-api.h"

#include    "ep2k-signals.h"
#include    "pantograph.h"
#include    "protective-device.h"
#include    "trac-motor.h"
#include    "dc-pulse-converter.h"
#include    "traction-regulator.h"
#include    "electric-brake-regulator.h"
#include    "driver-controller.h"

#include    "registrator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP2K : public Vehicle
{
public:

    EP2K(QObject *parent = Q_NULLPTR);

    ~EP2K();

private:

    enum
    {
        NUM_PANTS = 2,
        PANT_FWD = 0,
        PANT_BWD = 1
    };

    ProtectiveDevice    *fast_switch;

    bool bv_return;

    double ip;

    Registrator     *reg;

    DCPulseConverter *field_conv;

    TractionRegulator *trac_reg;

    double tracForce;

    EDBRegulator *brake_reg;

    DriverController *km;

    std::array<Pantograph *, NUM_PANTS> pantogrph;

    enum
    {
        NUM_MOTORS = 6
    };


    std::array<TractionMotor *, NUM_MOTORS> motor;

    std::array<DCPulseConverter *, NUM_MOTORS> trac_conv;

    void keyProcess();

    void initialization();

    void initHighVoltageSide();

    void initTracDrive();

    void step(double t, double dt);

    void stepHighVoltageSide(double t, double dt);

    void stepTracDrive(double t, double dt);

    bool getHoldingCoilState();

    void stepSignals(double t, double dt);
};

#endif // EP2K_H
