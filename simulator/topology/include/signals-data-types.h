#ifndef SIGNALS_DATA_TYPES_H
#define SIGNALS_DATA_TYPES_H

#include <QByteArray>

#include    <vector>
#include    <topology-export.h>

class Signal;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT signals_data_t
{
    std::vector<Signal*> line_signals;
    std::vector<Signal*> enter_signals;
    std::vector<Signal*> route_signals;
    std::vector<Signal*> exit_signals;
    std::vector<Signal*> shunt_signals;

    QByteArray serialize();
    void deserialize(QByteArray& data);
};

#endif // SIGNALS_DATA_TYPES_H
