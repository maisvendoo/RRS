#ifndef STATIONS_HANDLER_H
#define STATIONS_HANDLER_H

#include <topology-types.h>

class QByteArray;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class StationsHandler final
{
public:
    StationsHandler() = default;
    ~StationsHandler() = default;

    bool load(QByteArray& stations_data);

private:
    void deserialize(QByteArray& data);

    /// Список станций
    topology_stations_list_t stations;
};

#endif // STATIONS_HANDLER_H
