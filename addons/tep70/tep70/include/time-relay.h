#ifndef     TIME_RELAY_H
#define     TIME_RELAY_H

#include    "device.h"
#include    "relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TimeRelay : public Device
{
public:

    TimeRelay(size_t num_contacts = 1, QObject *parent = Q_NULLPTR);

    ~TimeRelay();

private:

    Relay       *relay;
    Timer       *timer;
};

#endif // TIME_RELAY_H
