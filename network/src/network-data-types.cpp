#include "network-data-types.h"

#include <QBuffer>

network_data_t::network_data_t()
    : stype(STYPE_EMPTY_DATA)
    , data_size(0)
    , data()
{
}

QByteArray network_data_t::serialize()
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

void network_data_t::deserialize(QByteArray &data)
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
    , socket(Q_NULLPTR)
    , received_data()
{
}
