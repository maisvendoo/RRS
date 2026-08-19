#ifndef     SWITCH_STATE_H
#define     SWITCH_STATE_H

#include    "topology-export.h"

#include    <QByteArray>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT switch_command_t
{
    QString conn_name = "";
    std::int8_t switch_direction = 0;
    std::int8_t switch_ref_state = 0;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT switch_state_t
{
    QString name = "";
    int8_t state_fwd = 1;
    int8_t state_bwd = 1;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

#endif // SWITCH_STATE_H
