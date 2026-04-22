#ifndef     SWITCH_STATE_H
#define     SWITCH_STATE_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct switch_command_t
{
    QString conn_name = "";
    std::int8_t switch_direction = 0;
    std::int8_t switch_ref_state = 0;

    QByteArray serialize()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << conn_name;
        stream << switch_direction;
        stream << switch_ref_state;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> conn_name;
        stream >> switch_direction;
        stream >> switch_ref_state;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct switch_state_t
{
    QString name = "";
    int8_t state_fwd = 1;
    int8_t state_bwd = 1;

    QByteArray serialize()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << name;
        stream << state_fwd;
        stream << state_bwd;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> name;
        stream >> state_fwd;
        stream >> state_bwd;
    }
};

#endif // SWITCH_STATE_H
