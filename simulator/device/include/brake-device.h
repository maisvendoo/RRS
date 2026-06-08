#ifndef     BRAKE_DEVICE_H
#define     BRAKE_DEVICE_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeDevice : public Device
{
    Q_OBJECT

public:
    BrakeDevice(QObject* parent = nullptr);

    virtual ~BrakeDevice() override;

    virtual void init(double pBP, double pFL);
};

#endif // BRAKE_DEVICE_H
