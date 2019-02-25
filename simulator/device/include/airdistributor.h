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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef AirDistributor* (*GetAriDistrubutor)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AIR_DISTRIBUTOR(ClassName) \
    extern "C" Q_DECL_EXPORT AirDistributor *getAirDistrubutor() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AirDistributor *loadAirDistributor(QString lib_path);

#endif // AIR_DISTRIBUTOR_H
