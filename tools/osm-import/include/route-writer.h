//------------------------------------------------------------------------------
//
//      osm-import: запись маршрута RRS из OSM-цепочек
//
//------------------------------------------------------------------------------

#ifndef ROUTE_WRITER_H
#define ROUTE_WRITER_H

#include "osm-data.h"
#include "osm-geometry.h"

#include <string>
#include <vector>

namespace osm
{

//------------------------------------------------------------------------------
/// Именованная готовая траектория (локальные метры)
//------------------------------------------------------------------------------
struct Trajectory
{
    std::string name;
    std::vector<LocalPoint> points;
    double length_m = 0.0;
};

//------------------------------------------------------------------------------
/// Параметры генерации маршрута
//------------------------------------------------------------------------------
struct WriteOptions
{
    /// Упрощение полилиний, м (0 - не упрощать)
    double simplify_epsilon_m = 0.5;

    /// Минимальная длина траектории, м (мусор выбрасываем)
    double min_trajectory_length_m = 20.0;

    /// Префикс имён траекторий
    std::string name_prefix = "osm";

    /// Высота пути, м
    double track_height_m = 0.0;
};

//------------------------------------------------------------------------------
/// Конвертировать цепочки в траектории (проекция + упрощение)
//------------------------------------------------------------------------------
std::vector<Trajectory> chains_to_trajectories(const OsmData& data,
                                                const std::vector<Chain>& chains,
                                                const WriteOptions& options);

//------------------------------------------------------------------------------
/// Записать маршрут-заготовку в каталог route_dir:
///   description.xml, route-type, objects.ref (пустой),
///   route1.map (пустой), topology/trajectories/*.traj,
///   topology/topology.xml (пустая конфигурация)
//------------------------------------------------------------------------------
bool write_route(const std::string& route_dir,
                 const std::string& route_name,
                 const std::vector<Trajectory>& trajectories,
                 std::string* error,
                 double center_lat = 0.0,
                 double center_lon = 0.0);

} // namespace osm

#endif // ROUTE_WRITER_H
