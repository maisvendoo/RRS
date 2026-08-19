#include "network-data-types.h"

#include <QBuffer>
#include "lz4.h"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
network_data_t::network_data_t()
    : data_size(0)
    , stype(STYPE_EMPTY_DATA)
    , is_compression(false)
    , data()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray network_data_t::serialize()
{
    QByteArray tmp_data;
    QDataStream stream(&tmp_data, QIODevice::WriteOnly);

    is_compression = (data.size() > 250);
    while (is_compression)
    {
        QByteArray compressed_data(data.size(), Qt::Uninitialized);
        int compressed_size = LZ4_compress_default(data.data(), compressed_data.data(),
                                                   data.size(), compressed_data.size());
        if ((compressed_size == 0) || (compressed_size >= data.size()))
        {
            is_compression = false;
            break;
        }

        compressed_data.resize(compressed_size);

        // Ожидаемый размер пакета - собственно размер, затем тип пакета,
        // признак сжатия и размер до сжатия,
        // затем стандартная сериализация QByteArray - длина данных и сами данные.
        // Наши данные не превысят 2^32-2 байт, их длина сериализуется одним quint32
        data_size = sizeof(data_size) + sizeof(stype) + sizeof(is_compression) + sizeof(uint32_t) + sizeof(quint32) + compressed_data.size();

        stream << data_size;
        stream << stype;
        stream << is_compression;
        stream << static_cast<uint32_t>(data.size());
        stream << compressed_data;
        return tmp_data;
    }

    data_size = sizeof(data_size) + sizeof(stype) + sizeof(is_compression) + sizeof(quint32) + data.size();
    stream << data_size;
    stream << stype;
    stream << is_compression;
    stream << data;

    return tmp_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void network_data_t::deserialize(QByteArray &data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> data_size;
    stream >> stype;
    stream >> is_compression;
    if (is_compression)
    {
        uint32_t original_size;
        stream >> original_size;
        QByteArray compressed_data;
        stream >> compressed_data;

        this->data.resize(original_size);
        int decompressed_size = LZ4_decompress_safe(compressed_data.data(), this->data.data(),
                                                    compressed_data.size(), this->data.size());
        if (decompressed_size != static_cast<int>(original_size))
        {
            stype = STYPE_EMPTY_DATA;
        }
    }
    else
    {
        stream >> this->data;
    }
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
    , profile_update_interval(1.0)
    , profile_update_prev_time(0.0)
    , profile_backward(4000.0)
    , profile_forward(4000.0)
    , socket(nullptr)
    , received_data()
{
}
