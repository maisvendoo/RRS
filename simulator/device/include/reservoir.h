#ifndef     RESERVOIR_H
#define     RESERVOIR_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Reservoir : public Device
{
public:

    Reservoir(double volume, QObject *parent = Q_NULLPTR);

    ~Reservoir();

private:

    double  V;
    double  Q;
};

#endif // RESERVOIR_H
