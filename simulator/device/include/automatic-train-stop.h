#ifndef     AUTOMATIC_TRAIN_STOP_H
#define     AUTOMATIC_TRAIN_STOP_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AutoTrainStop : public BrakeDevice
{
public:

    AutoTrainStop(QObject *parent = Q_NULLPTR);

    virtual ~AutoTrainStop();

protected:


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
