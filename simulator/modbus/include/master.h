#ifndef     MASTER_H
#define     MASTER_H

#include    <QObject>
#include    <QModbusClient>
#include    <QtSerialPort/QSerialPortInfo>

#include    "rs485.h"
#include    "slave.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Master : public QObject
{
public:

    Master(QObject *parent = Q_NULLPTR);

    ~Master();

    bool init(QString cfg_path);

    void readDiscreteInputsRequest(Slave *slave);

    void readInputRegistersRequest(Slave *slave);

    void writeCoils(Slave *slave);

    void writeHoldingRegisters(Slave *slave);

    void writeHoldingRegister(Slave* slave, slave_data_t hreg);

private:

    QModbusClient           *modbusDevice;

    port_config_t           port_config;

public:

    QMap<quint16, Slave *>  slave;

private:

    enum
    {
        MASTER_INFO = 0,
        MASTER_WARNING = 1,
        MASTER_ERROR = 2
    };    


    bool loadPortConfig(const QString &path, port_config_t &port_config);

    bool loadNetworkMap(const QString &path);

    bool serialConnection(port_config_t port_config);


private slots:

    void slotErrorModbus(QModbusDevice::Error error);

    void slotStateModbus(QModbusDevice::State state);
};

#endif // MASTER_H
