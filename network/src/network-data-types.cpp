#include "network-data-types.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

network_data_t::network_data_t()
    : stype(STYPE_EMPTY_DATA)
    , data_size(0)
    , data()
{
}

QByteArray network_data_t::serialize()
{
    QByteArray tmp_data;
    QDataStream stream(&tmp_data, QIODevice::WriteOnly);

    stream << data.size() + sizeof(data_size) + sizeof(stype);
    stream << stype;
    stream << data;

    return tmp_data;
}

void network_data_t::deserialize(QByteArray &data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

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
    , socket(nullptr)
    , received_data()
{
}
