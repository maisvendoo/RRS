#include    "master.h"

#include    "CfgReader.h"

#include    <QVariant>
#include    <QModbusRtuSerialMaster>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Master::Master(QObject *parent) : QObject(parent)
  , modbusDevice(Q_NULLPTR)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Master::~Master()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Master::init(QString cfg_path)
{
    loadPortConfig(cfg_path, port_config);

    if (modbusDevice != Q_NULLPTR)
    {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = Q_NULLPTR;
    }

    modbusDevice = new QModbusRtuSerialMaster(this);

    if (!modbusDevice)
    {

        return false;
    }

    connect(modbusDevice, &QModbusClient::errorOccurred, this, &Master::slotErrorModbus);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &Master::slotStateModbus);

    return serialConnection(port_config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Master::loadPortConfig(const QString &path, port_config_t &port_config)
{
    CfgReader cfg;

    if (!cfg.load(path))
    {

        return false;
    }

    QString secName = "SerialPort";

    cfg.getString(secName, "Name", port_config.name);
    cfg.getInt(secName, "Baudrate", port_config.baudrate);
    cfg.getInt(secName, "DataBits", port_config.data_bits);
    cfg.getInt(secName, "StopBits", port_config.stop_bits);
    cfg.getInt(secName, "Parity", port_config.parity);
    cfg.getInt(secName, "Timeout", port_config.timeout);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Master::serialConnection(port_config_t port_config)
{
    if (modbusDevice == Q_NULLPTR)
    {

        return false;
    }

    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, port_config.name);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, port_config.baudrate);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, port_config.data_bits);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, port_config.stop_bits);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, port_config.parity);
        modbusDevice->setTimeout(port_config.timeout);

        if (!modbusDevice->connectDevice())
        {
            return false;
        }


        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::slotErrorModbus(QModbusDevice::Error error)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::slotStateModbus(QModbusDevice::State state)
{

}

