#ifndef     SIGNAL_COMMAND_H
#define     SIGNAL_COMMAND_H

#include    "topology-export.h"

#include    <QByteArray>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT signal_command_t
{
    QString conn_name = "";
    std::int8_t sig_dir = 0;
    bool command_open_train = false;
    bool command_open_shunting = false;
    bool command_open_call = false;
    bool command_close = false;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

#endif // SIGNAL_COMMAND_H
