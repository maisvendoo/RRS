//------------------------------------------------------------------------------
//
//      osm-import: проекция WGS84 -> локальные метры, упрощение
//
//------------------------------------------------------------------------------

#ifndef OSM_GEOMETRY_H
#define OSM_GEOMETRY_H

#include "osm-data.h"

#include <cmath>
#include <string>
#include <vector>

namespace osm
{

//------------------------------------------------------------------------------
/// Точка в локальной метрической системе координат RRS:
/// X - на восток, Y - на север, Z - вверх (высота)
//------------------------------------------------------------------------------
struct LocalPoint
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

//------------------------------------------------------------------------------
/// Проекция: equirectangular вокруг начала координат (lat0, lon0).
/// Для областей до ~100 км искажение пренебрежимо
//------------------------------------------------------------------------------
struct Projector
{
    double lat0 = 0.0;
    double lon0 = 0.0;
    double meters_per_deg_lat = 111132.0;
    double meters_per_deg_lon = 0.0;

    Projector() = default;

    Projector(double lat_origin, double lon_origin)
    {
        init(lat_origin, lon_origin);
    }

    void init(double lat_origin, double lon_origin)
    {
        lat0 = lat_origin;
        lon0 = lon_origin;
        meters_per_deg_lat = 111132.0;
        meters_per_deg_lon = 111320.0 * std::cos(lat0 * 0.017453292519943295);
    }

    LocalPoint toLocal(double lat, double lon) const
    {
        LocalPoint p;
        p.x = (lon - lon0) * meters_per_deg_lon;
        p.y = (lat - lat0) * meters_per_deg_lat;
        return p;
    }

};

//------------------------------------------------------------------------------
/// Упрощение ломаной (Douglas-Peucker, эпсилон в метрах)
//------------------------------------------------------------------------------
std::vector<LocalPoint> simplify_polyline(const std::vector<LocalPoint>& points,
                                          double epsilon_m);

//------------------------------------------------------------------------------
/// Длина ломаной, м
//------------------------------------------------------------------------------
double polyline_length(const std::vector<LocalPoint>& points);

} // namespace osm

#endif // OSM_GEOMETRY_H
