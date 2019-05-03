#ifndef     LOCO_CRANE_H
#define     LOCO_CRANE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT LocoCrane : public BrakeDevice
{
public:

    LocoCrane(QObject *parent = Q_NULLPTR);

    virtual ~LocoCrane();

    void setFeedlinePressure(double pFL);

    void setBrakeCylinderPressure(double pBC);

    void setAirDistributorFlow(double Qvr);

protected:

    double pFL;

    double pBC;

    double Qvr;
};

#endif // LOCO_CRANE_H
