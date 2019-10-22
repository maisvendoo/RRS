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

    /// Флаг передачи данных
    bool    is_transmit;

    void controlSignalsProcess();

    void feedbackSignalsProcess();

     slave_data_t *searchByIndex(size_t index, data_map_t &data);
};

#endif // MODBUS_H
