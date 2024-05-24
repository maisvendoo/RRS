#ifndef     FREEJOY_H
#define     FREEJOY_H

#include    "virtual-interface-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FreeJoy : public VirtualInterfaceDevice
{
public:

    FreeJoy(QObject *parent = Q_NULLPTR);

    ~FreeJoy();

    bool init(QString cfg_path);

    void process();

private:

};

#endif
