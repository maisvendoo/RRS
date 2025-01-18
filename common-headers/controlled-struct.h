#ifndef     CLIENT_CONTROL_STRUCT_H
#define     CLIENT_CONTROL_STRUCT_H

#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct controlled_t
{
    int controlled_vehicle = -1;
    int current_vehicle = -1;
    std::vector<int> pressed_keys;

    controlled_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << controlled_vehicle;
        stream << current_vehicle;
        stream << (quint32)pressed_keys.size();

        for (auto key_id : pressed_keys)
        {
            stream << key_id;
        }

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> controlled_vehicle;
        stream >> current_vehicle;

        quint32 num;
        stream >> num;

        pressed_keys.clear();
        pressed_keys.resize(num);

        for (size_t i = 0; i < pressed_keys.size(); ++i)
        {
            stream >> pressed_keys[i];
        }
    }
};

#endif // CLIENT_CONTROL_STRUCT_H
