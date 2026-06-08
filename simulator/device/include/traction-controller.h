#ifndef     TRACTION_CONTROLLER_H
#define     TRACTION_CONTROLLER_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT  TractionController : public Device
{
public:

    TractionController(QObject *parent = nullptr);

    virtual ~TractionController();

protected:


};

#endif // TRACTION_CONTROLLER_H
