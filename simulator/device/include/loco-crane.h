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

    virtual void setHandlePosition(double pos);

    double getBrakeCylinderFlow() const;

    virtual double getHandlePosition() const = 0;

    virtual double getHandleShift() const = 0;

    virtual double getAirDistribPressure() const;

    virtual void stepSound();

    void release(bool is_release);

protected:

    double pFL;

    double pBC;

    double Qvr;

    double Qbc;

    double pos;

    double is_release;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef LocoCrane* (*GetLocoCrane)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_LOCO_CRANE(ClassName) \
    extern "C" Q_DECL_EXPORT LocoCrane *getLocoCrane() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT LocoCrane *loadLocoCrane(QString lib_path);

#endif // LOCO_CRANE_H
