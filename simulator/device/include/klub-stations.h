#ifndef KLUB_STATIONS_H
#define KLUB_STATIONS_H

#include    <vec3.h>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct station_t
{
    dvec3 coord = {0.0, 0.0, 0.0};
    QString name = "";

    station_t()
    {

    }
};

#endif // KLUB_STATIONS_H
