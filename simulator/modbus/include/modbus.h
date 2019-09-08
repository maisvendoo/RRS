#ifndef     MODBUS_H
#define     MODBUS_H

#include    "virtual-interface-device.h"

#include    "master.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Modbus : public VirtualInterfaceDevice
{
public:

    Modbus(QObject *parent = Q_NULLPTR);

    ~Modbus();

    bool init(QString cfg_path);

    void process();

private:

    /// Мастер-устройство
    Master  *master;
};

#endif // MODBUS_H
