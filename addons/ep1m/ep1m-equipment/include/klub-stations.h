#ifndef KLUB_STATIONS_H
#define KLUB_STATIONS_H

#include    <vec3.h>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct klub_station_t
{
    dvec3 coord = {0.0, 0.0, 0.0};
    QString name = "";

    klub_station_t()
    {
        coord.x = 0.0;
        coord.y = 0.0;
        coord.z = 0.0;
        name = QString();
    }
};

#endif // KLUB_STATIONS_H
