#ifndef     AUTOMATIC_TRAIN_STOP_H
#define     AUTOMATIC_TRAIN_STOP_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT AutoTrainStop : public BrakeDevice
{
public:

    AutoTrainStop(QObject *parent = Q_NULLPTR);

    virtual ~AutoTrainStop();

    void powerOn(bool on);

    void keyOn(bool on);

    void setFeedlinePressure(double pFL);

    void setBrakepipePressure(double pTM);

    virtual double getEmergencyBrakeRate() const;

    bool getState();


protected:

    double is_powered;

    double is_key_on;

    double pFL;

    double pTM;

    bool state;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef AutoTrainStop* (*GetAutoTrainStop)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTO_TRAIN_STOP(ClassName) \
    extern "C" Q_DECL_EXPORT AutoTrainStop *getAutoTrainStop() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AutoTrainStop *loadAutoTrainStop(QString lib_path);

#endif // AUTOMATIC_TRAIN_STOP_H
