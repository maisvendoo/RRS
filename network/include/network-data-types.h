#ifndef     NETWORK_DATA_TYPES_H
#define     NETWORK_DATA_TYPES_H

#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>
#include    <QTcpSocket>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StructureType
{
    STYPE_EMPTY_DATA,

    STYPE_REQUEST_ROUTE_INFO,
    STYPE_REQUEST_TOPOLOGY_DATA,
//    STYPE_REQUEST_TOPOLOGY_UPDATE,

    STYPE_REQUEST_SIGNALS_DATA,
//    STYPE_REQUEST_SIGNALS_UPDATE,

    STYPE_REQUEST_VEHICLES_INFO,
    STYPE_REQUEST_VEHICLES_POS_UPDATE,
    STYPE_REQUEST_VEHICLES_STATE_UPDATE,

    STYPE_COMMAND_SWITCH_STATE,
    STYPE_COMMAND_OPEN_SIGNAL,
    STYPE_COMMAND_CLOSE_SIGNAL,
    STYPE_COMMAND_VEHICLE_CONTROL,

    STYPE_ROUTE_INFO,
    STYPE_TOPOLOGY_DATA,
//    STYPE_TOPOLOGY_STATE,
    STYPE_SWITCH_UPDATE,
    STYPE_TRAJ_BUSY_UPDATE,

    STYPE_SIGNALS_DATA,
//    STYPE_SIGNALS_STATE,
    STYPE_SIGNAL_UPDATE,

    STYPE_VEHICLES_INFO,
    STYPE_VEHICLES_POS_UPDATE,
    STYPE_VEHICLES_UPDATE,
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct network_data_t
{
    /// Тип передаваемой/принимаемой структуры
    StructureType   stype = STYPE_EMPTY_DATA;
    /// Размер данных
    qsizetype data_size = 0;
    /// Сериализованные данные
    QByteArray      data;

    /// Сериализуем, подготоваливая кадр, передаваемый по сети
    QByteArray serialize()
    {
        QByteArray tmp_data;
        QBuffer buff(&tmp_data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << data.size() + sizeof(data_size) + sizeof(stype);
        stream << stype;
        stream << data;

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> data_size;
        stream >> stype;
        stream >> this->data;

        // Контрольная сериализация полученных данных
        QByteArray tmp = this->serialize();
        // Удаляем из полученного блока фактически сериализованное
        data = data.mid(tmp.size());
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct client_data_t
{
    int id = 0;
    QTcpSocket  *socket = Q_NULLPTR;
    network_data_t  received_data;
};

#endif
