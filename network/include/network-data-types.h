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
    STYPE_TOPOLOGY_DATA,
    STYPE_TRAIN_POSITION,
    STYPE_CONNECTOR_STATE,
    STYPE_TRAJ_BUSY_STATE,
    STYPE_SIGNALS_LIST,
    STYPE_SIGNAL_STATE
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
    QTcpSocket  *socket = Q_NULLPTR;
    network_data_t  received_data;
};

#endif
