#include    "master.h"

#include    "CfgReader.h"

#include    <QVariant>
#include    <QModbusRtuSerialMaster>
#include    <QModbusDataUnit>
#include    <QModbusReply>
#include    <QDir>
#include    <QFileInfo>

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
    loadPortConfig(cfg_path + QDir::separator() + "rs485.xml", port_config);

    if (!loadNetworkMap(cfg_path + QDir::separator() + "modbus-map.xml"))
    {
        return false;
    }

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
bool Master::loadNetworkMap(const QString &path)
{
    if (path.isEmpty())
    {
        return false;
    }

    CfgReader cfg;

    if (!cfg.load(path))
    {
        return false;
    }

    QFileInfo info(path);
    QDir cfg_dir = info.dir();

    QDomNode secNode = cfg.getFirstSection("Slave");

    while (!secNode.isNull())
    {
        Slave *slave = new Slave();

        QString config_name = "";
        cfg.getString(secNode, "Config", config_name);

        if (slave->load_config(cfg_dir.path() + QDir::separator() + config_name + ".xml"))
        {
            int tmp = 0;
            if (cfg.getInt(secNode, "id", tmp))
            {
                slave->id = static_cast<quint16>(tmp);
            }

            cfg.getString(secNode, "Description", slave->description);
            cfg.getBool(secNode, "ConfigRequired", slave->is_config_required);

            if (cfg.getInt(secNode, "CellsMask", tmp))
            {
                slave->cells_mask = static_cast<quint16>(tmp);
            }

            this->slave.insert(slave->id, slave);
        }

        secNode = cfg.getNextSection();
    }

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
void Master::readDiscreteInputsRequest(Slave *slave)
{
    if (!slave->isConnected())
        return;

    quint16 addr = slave->discrete_input.begin().value().address;
    QModbusDataUnit unit(QModbusDataUnit::DiscreteInputs, addr, static_cast<quint16>(slave->discrete_input.size()));

    QModbusReply *reply = modbusDevice->sendReadRequest(unit, slave->id);

    if (reply != Q_NULLPTR)
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, slave, &Slave::slotReadDiscreteInputs, Qt::QueuedConnection);

            connect(reply, &QModbusReply::errorOccurred, this, &Master::slotErrorModbus);
        }
        else
        {
            reply->deleteLater();
        }
    }
    else
    {
        slave->incErrosCount();        
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::readInputRegistersRequest(Slave *slave)
{
    if (!slave->isConnected())
        return;

    quint16 addr = slave->input_register.begin().value().address;
    QModbusDataUnit unit(QModbusDataUnit::InputRegisters, addr, static_cast<quint16>(slave->input_register.size()));

    QModbusReply *reply = modbusDevice->sendReadRequest(unit, slave->id);

    if (reply != Q_NULLPTR)
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, slave, &Slave::slotReadInputRegisters, Qt::QueuedConnection);

            connect(reply, &QModbusReply::errorOccurred, this, &Master::slotErrorModbus);
        }
        else
        {
            reply->deleteLater();
        }
    }
    else
    {
        slave->incErrosCount();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::writeCoils(Slave *slave)
{
    if (!slave->isConnected())
        return;

    quint16 addr = slave->coil.begin().value().address;
    QModbusDataUnit unit(QModbusDataUnit::Coils, addr, static_cast<quint16>(slave->coil.size()));

    for (int i = 0; i < slave->coil.size(); ++i)
    {
        unit.setValue(i, static_cast<quint16>(slave->coil[i].value));
    }

    QModbusReply *reply = modbusDevice->sendWriteRequest(unit, slave->id);

    if (reply != Q_NULLPTR)
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, slave, &Slave::slotWrited);

            connect(reply, &QModbusReply::errorOccurred, this, &Master::slotErrorModbus);
        }
        else
        {
            reply->deleteLater();
        }
    }
    else
    {
        slave->incErrosCount();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::writeHoldingRegisters(Slave *slave)
{
    if (!slave->isConnected())
        return;

    quint16 addr = slave->holding_register.begin().value().address;
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, addr, static_cast<quint16>(slave->holding_register.size()));

    for (int i = 0; i < slave->holding_register.size(); ++i)
    {
        unit.setValue(i, static_cast<quint16>(slave->holding_register[i].value));
    }

    QModbusReply *reply = modbusDevice->sendWriteRequest(unit, slave->id);

    if (reply != Q_NULLPTR)
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, slave, &Slave::slotWrited);

            connect(reply, &QModbusReply::errorOccurred, this, &Master::slotErrorModbus);
        }
        else
        {
            reply->deleteLater();
        }
    }
    else
    {
        slave->incErrosCount();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::slotErrorModbus(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError)
        return;

    QString error_msg = modbusDevice->errorString();

    switch (error)
    {
    case QModbusDevice::NoError:

        break;

    case QModbusDevice::ReadError:

        break;

    case QModbusDevice::WriteError:

        break;

    case QModbusDevice::ConnectionError:

        break;

    case QModbusDevice::ConfigurationError:

        break;

    case  QModbusDevice::TimeoutError:
        {
            QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
            quint16 id = static_cast<quint16>(reply->serverAddress());

            slave[id]->incErrosCount();

            break;
        }

    case QModbusDevice::ProtocolError:

        break;

    case QModbusDevice::UnknownError:

        break;

    case QModbusDevice::ReplyAbortedError:

        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Master::slotStateModbus(QModbusDevice::State state)
{
    switch (state)
    {
    case QModbusDevice::State::ConnectingState:

        break;

    case QModbusDevice::State::ConnectedState:

        break;

    case QModbusDevice::State::UnconnectedState:

        break;

    case QModbusDevice::State::ClosingState:

        break;
    }
}

