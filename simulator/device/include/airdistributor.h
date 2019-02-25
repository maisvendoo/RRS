#ifndef     AIR_DISTRIBUTOR_H
#define     AIR_DISTRIBUTOR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AirDistributor : public Device
{
public:

    AirDistributor(QObject *parent = Q_NULLPTR);

    virtual ~AirDistributor();

    void setBrakepipePressure(double pTM);

    void setBrakeCylinderPressure(double pBC);

    double getBrakeCylinderAirFlow() const;

protected:

    double  pTM;

    double  pBC;

    double  Qbc;
};

#endif // AIR_DISTRIBUTOR_H
