#ifndef     TRACTION_CONTROLLER_H
#define     TRACTION_CONTROLLER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT  TractionController : public Device
{
public:

    TractionController(QObject *parent = Q_NULLPTR);

    virtual ~TractionController();

protected:


};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef  TractionController* (*GetTractionController)();

#define GET_TRACTION_CONTROLLER(ClassName) \
    extern "C" Q_DECL_EXPORT TractionController *getTractionController() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT TractionController *loadTractionController(QString lib_path);

#endif // TRACTION_CONTROLLER_H
