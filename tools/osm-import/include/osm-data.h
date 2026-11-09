//------------------------------------------------------------------------------
//
//      osm-import: данные OSM (ноды/веи) и утилиты склейки путей
//
//------------------------------------------------------------------------------

#ifndef OSM_DATA_H
#define OSM_DATA_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace osm
{

//------------------------------------------------------------------------------
/// Точка OSM (WGS84)
//------------------------------------------------------------------------------
struct Node
{
    double lat = 0.0;
    double lon = 0.0;
    std::uint64_t id = 0;
};

//------------------------------------------------------------------------------
/// Линейный объект OSM (way)
//------------------------------------------------------------------------------
struct Way
{
    std::uint64_t id = 0;
    std::vector<std::uint64_t> nodes;
    /// railway=* (rail, light_rail, tram, subway, siding, ...)
    std::string railway;
    /// usage=* (main, branch, industrial, ...)
    std::string usage;
    /// service=* (siding, yard, spur, crossover...)
    std::string service;
    /// name=*
    std::string name;
    /// gauge=*
    std::string gauge;
    /// electrified=* (contact_line, rail, no)
    std::string electrified;
};

//------------------------------------------------------------------------------
/// Распарсенные данные
//------------------------------------------------------------------------------
struct OsmData
{
    std::map<std::uint64_t, Node> nodes;
    std::vector<Way> ways;

    /// bounding box запроса
    double min_lat = 0.0;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double max_lon = 0.0;
};

//------------------------------------------------------------------------------
/// Склеенная цепочка нод (траектория в терминах OSM): узлы степени 2
/// объединены в непрерывные ломаные. Точки конца имеют степень != 2
/// (тупик, стрелка, примыкание)
//------------------------------------------------------------------------------
struct Chain
{
    std::vector<std::uint64_t> node_ids;
    /// Метаданные от самых частых way-участников
    std::string railway;
    std::string service;
    std::string name;
};

//------------------------------------------------------------------------------
/// Прочитать .osm XML из файла или строки
//------------------------------------------------------------------------------
bool parse_osm_xml(const std::string& content, OsmData& out, std::string* error);
bool parse_osm_file(const std::string& path, OsmData& out, std::string* error);

//------------------------------------------------------------------------------
/// Скачать железные дороги по bbox с Overpass API
/// (bbox: min_lat,min_lon,max_lat,max_lon)
//------------------------------------------------------------------------------
bool download_overpass(const std::string& overpass_url,
                       double min_lat, double min_lon,
                       double max_lat, double max_lon,
                       std::string& xml_out, std::string* error);

//------------------------------------------------------------------------------
/// Отфильтровать веи по железнодорожным критериям.
/// include_service - включать подъездные/станционные (service=*)
//------------------------------------------------------------------------------
std::vector<Way> filter_railways(const OsmData& data, bool include_service);

//------------------------------------------------------------------------------
/// Склеить отфильтрованные веи в цепочки (траектории)
//------------------------------------------------------------------------------
std::vector<Chain> build_chains(const OsmData& data,
                                const std::vector<Way>& ways);

} // namespace osm

#endif // OSM_DATA_H
