#include "network-data-types.h"

#include <QBuffer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
network_data_t::network_data_t()
    : stype(STYPE_EMPTY_DATA)
    , data_size(0)
    , data()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray network_data_t::serialize()
{
    QByteArray tmp_data;
    QBuffer buff(&tmp_data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    // Ожидаемый размер пакета - поле под собственно размер, затем тип пакета,
    // затем стандратная сериализация QByteArray - длина данных и сами данные.
    // Наши данные не превысят 2^32-2 байт, их длина сериализуется одним quint32
    stream << sizeof(data_size) + sizeof(stype) + sizeof(quint32) + data.size();
    stream << stype;
    stream << data;

    return buff.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void network_data_t::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> data_size;
    stream >> stype;
    stream >> this->data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
client_data_t::client_data_t()
    : id(0)
    , pos_update_interval(0.0)
    , pos_update_prev_time(0.0)
    , state_update_interval(0.0)
    , state_update_prev_time(0.0)
    , controlled_update_interval(0.0)
    , controlled_update_prev_time(0.0)
    , players_update_interval(0.0)
    , players_update_prev_time(0.0)
    , socket(nullptr)
    , received_data()
{
}
