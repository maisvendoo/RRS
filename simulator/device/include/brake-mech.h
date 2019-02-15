//------------------------------------------------------------------------------
//
//      Vehicle brake mechanism abstract class
//      (c) maisvendoo, 15/02/2019
//
//------------------------------------------------------------------------------
#ifndef     BRAKE_MECH_H
#define     BRAKE_MECH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeMech : public Device
{
public:

    BrakeMech(QObject *parent = Q_NULLPTR);

    ~BrakeMech();

    void setAxisNumber(int numAxis);

    double getBrakeTorque() const;

protected:

    /// Number of vehicle axis
    int     numAxis;

    /// Axis brake torque
    double  brakeTorque;

private:

    virtual void load_config(CfgReader &cfg);
};

#endif // BRAKEMECH_H
