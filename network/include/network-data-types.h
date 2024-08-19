#ifndef     NETWORK_DATA_TYPES_H
#define     NETWORK_DATA_TYPES_H

#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum StructureType
{
    STYPE_EMPTY_DATA,
    STYPE_TOPOLOGY_DATA,
    STYPE_TRAIN_POSITION,
    STYPE_CONNECTOR_STATE
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
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << stype;
        stream << data.size() + sizeof(stype) + sizeof(data_size);
        stream << data;

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> stype;
        stream >> data_size;
        stream >> data;
    }
};

#endif
