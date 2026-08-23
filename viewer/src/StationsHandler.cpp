#include "StationsHandler.h"

#include "Logger.h"

#include <QDataStream>
#include <QIODevice>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool StationsHandler::load(QByteArray& stations_data)
{
    deserialize(stations_data);

    LOG_INFO("Loaded %lld stations", static_cast<long long>(stations.size()));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void StationsHandler::deserialize(QByteArray& data)
{
    // Очищаем список станций
    stations.clear();

    QDataStream stream(&data, QIODevice::ReadOnly);

    uint32_t stations_count = 0;
    stream >> stations_count;

    for (uint32_t i = 0; i < stations_count; ++i)
    {
        QByteArray station_data;
        stream >> station_data;

        topology_station_t station;
        station.deserialize(station_data);
        stations.push_back(station);
    }
}
