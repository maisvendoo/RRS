#ifndef     SWITCH_STATE_H
#define     SWITCH_STATE_H

#include    <QString>
#include    <QBuffer>

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
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << conn_name;
        stream << switch_direction;
        stream << switch_ref_state;

        return buff.data();
    }

    void deserialize(QByteArray data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

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
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << name;
        stream << state_fwd;
        stream << state_bwd;

        return buff.data();
    }

    void deserialize(QByteArray data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> name;
        stream >> state_fwd;
        stream >> state_bwd;
    }
};

#endif
