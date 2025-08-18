#ifndef CLIENT_CONTROL_STRUCT_H
#define CLIENT_CONTROL_STRUCT_H

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

#include <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct controlled_t final
{
    std::uint16_t controlled_vehicle = 0;
    std::uint16_t current_vehicle = 0;
    std::uint16_t controlled_cabine_idx = 0;
    bool need_debug_msg = false;
    std::vector<std::uint16_t> pressed_keys;

    QByteArray serialize() const
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << controlled_vehicle;
        stream << current_vehicle;
        stream << controlled_cabine_idx;
        stream << need_debug_msg;
        stream << static_cast<std::uint16_t>(pressed_keys.size());

        for (auto key_id : pressed_keys)
        {
            stream << key_id;
        }

        return buff.data();
    }

    void deserialize(QByteArray& data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> controlled_vehicle;
        stream >> current_vehicle;
        stream >> controlled_cabine_idx;
        stream >> need_debug_msg;

        std::uint16_t num;
        stream >> num;

        pressed_keys.clear();
        pressed_keys.resize(num);

        for (std::uint16_t i = 0; i < num; ++i)
        {
            stream >> pressed_keys[i];
        }
    }
};

#endif // CLIENT_CONTROL_STRUCT_H
