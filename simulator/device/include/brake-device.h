#ifndef     BRAKE_DEVICE_H
#define     BRAKE_DEVICE_H

#include    "device.h"

class BrakeDevice : public Device
{
public:

    BrakeDevice(QObject *parent = Q_NULLPTR);

    virtual ~BrakeDevice();

    virtual void init(double pTM, double pFL);

protected:


};

#endif // BRAKE_DEVICE_H
