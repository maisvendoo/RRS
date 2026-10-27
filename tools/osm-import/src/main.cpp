//------------------------------------------------------------------------------
//
//      osm-import: импорт железных дорог OpenStreetMap / GPX в маршрут RRS
//      + высоты рельефа из SRTM .hgt (по мотивам TSRE5: MapDataOSM,
//      CoordsGpx, GeoHgtFile)
//
//      Источник (один из):
//        --bbox min_lat,min_lon,max_lat,max_lon   скачать с Overpass API
//        --file путь/к/file.osm                   локальный XML OSM
//        --gpx путь/к/track.gpx                   треки GPX -> пути
//      Опции:
//        --out КАТ       каталог маршрута (обязательно)
//        --hgt ПУТЬ      файл или каталог SRTM .hgt: высоты путей
//        --overpass URL  зеркало Overpass (по умолчанию overpass-api.de)
//        --simplify М    упрощение полилиний, м (0.5)
//        --minlen М      минимальная длина траектории, м (20)
//        --prefix ИМЯ    префикс имён траекторий (osm)
//        --service       включать станционные/подъездные пути
//        --name ИМЯ      имя маршрута (description.xml)
//
//------------------------------------------------------------------------------

#include "osm-data.h"
#include "osm-geometry.h"
#include "route-writer.h"
#include "gpx-reader.h"
#include "geo-height.h"

#include <QString>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static double total_length(const std::vector<osm::Trajectory>& list)
{
    double sum = 0.0;
    for (const auto& t : list)
    {
        sum += t.length_m;
    }
    return sum;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static int import_gpx(const std::string& gpx_file,
                      const std::string& hgt_path,
                      const std::string& out_dir,
                      const std::string& route_name,
                      const osm::WriteOptions& options)
{
    std::vector<osm::GpxTrack> tracks;
    std::string error = "";

    std::printf("Parsing GPX: %s\n", gpx_file.c_str());

    if (!osm::parse_gpx(gpx_file, tracks, &error))
    {
        std::fprintf(stderr, "FAILED: %s\n", error.c_str());
        return 1;
    }

    std::printf("GPX tracks: %zu\n", tracks.size());

    osm::GeoHeight geo(hgt_path);
    if (geo.hasData())
    {
        std::printf("SRTM heights: loaded\n");
    }

    // Bbox по всем трекам, центр - начало координат
    double min_lat = 1e9;
    double max_lat = -1e9;
    double min_lon = 1e9;
    double max_lon = -1e9;

    for (const osm::GpxTrack& track : tracks)
    {
        for (const osm::GpxPoint& p : track.points)
        {
            min_lat = std::min(min_lat, p.lat);
            max_lat = std::max(max_lat, p.lat);
            min_lon = std::min(min_lon, p.lon);
            max_lon = std::max(max_lon, p.lon);
        }
    }

    const double lat0 = 0.5 * (min_lat + max_lat);
    const double lon0 = 0.5 * (min_lon + max_lon);
    const double m_lat = 111132.0;
    const double m_lon = 111320.0 * std::cos(lat0 * 0.017453292519943295);

    std::vector<osm::Trajectory> trajectories;
    int index = 1;

    for (const osm::GpxTrack& track : tracks)
    {
        std::vector<osm::LocalPoint> points;
        points.reserve(track.points.size());

        for (const osm::GpxPoint& p : track.points)
        {
            osm::LocalPoint lp;
            lp.x = (p.lon - lon0) * m_lon;
            lp.y = (p.lat - lat0) * m_lat;

            if (p.has_ele)
            {
                lp.z = p.ele;
            }
            else if (geo.hasData())
            {
                lp.z = geo.height(p.lat, p.lon);
            }
            else
            {
                lp.z = 0.0;
            }

            points.push_back(lp);
        }

        // Убираем дубли подряд
        std::vector<osm::LocalPoint> filtered;
        for (const auto& p : points)
        {
            if (!filtered.empty())
            {
                const double dx = p.x - filtered.back().x;
                const double dy = p.y - filtered.back().y;
                if (dx * dx + dy * dy < 0.01)
                {
                    continue;
                }
            }
            filtered.push_back(p);
        }

        if (options.simplify_epsilon_m > 0.0 && filtered.size() > 2)
        {
            filtered = osm::simplify_polyline(filtered, options.simplify_epsilon_m);
        }

        osm::Trajectory traj;
        traj.points = filtered;
        traj.length_m = osm::polyline_length(filtered);

        if (traj.length_m < options.min_trajectory_length_m)
        {
            continue;
        }

        char buf[64];
        std::snprintf(buf, sizeof(buf), "%s_%04d",
                      options.name_prefix.c_str(), index++);
        traj.name = buf;

        trajectories.push_back(std::move(traj));
    }

    std::printf("Trajectories: %zu, total %.1f km\n",
                trajectories.size(), total_length(trajectories) / 1000.0);

    if (!osm::write_route(out_dir, route_name, trajectories, &error,
                          lat0, lon0))
    {
        std::fprintf(stderr, "FAILED: %s\n", error.c_str());
        return 1;
    }

    std::printf("Route written to %s\n", out_dir.c_str());
    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    std::string overpass_url = "https://overpass-api.de/api/interpreter";
    std::string out_dir = "";
    std::string osm_file = "";
    std::string route_name = "OSM route";
    std::string bbox_str = "";
    std::string gpx_file = "";
    std::string hgt_path = "";

    osm::WriteOptions options;
    bool include_service = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        auto next = [&]() -> std::string
        {
            return (i + 1 < argc) ? argv[++i] : "";
        };

        if (arg == "--out") out_dir = next();
        else if (arg == "--file") osm_file = next();
        else if (arg == "--bbox") bbox_str = next();
        else if (arg == "--gpx") gpx_file = next();
        else if (arg == "--hgt") hgt_path = next();
        else if (arg == "--overpass") overpass_url = next();
        else if (arg == "--name")
        {
            // Windows-консоль отдаёт argv в OEM-кодировке:
            // перекодируем в UTF-8 (так пишем в description.xml)
            const std::string raw = next();
            route_name = QString::fromLocal8Bit(raw.c_str()).toStdString();
        }
        else if (arg == "--prefix") options.name_prefix = next();
        else if (arg == "--simplify") options.simplify_epsilon_m = std::stod(next());
        else if (arg == "--minlen") options.min_trajectory_length_m = std::stod(next());
        else if (arg == "--service") include_service = true;
        else
        {
            std::fprintf(stderr, "Unknown argument: %s\n", arg.c_str());
            return 1;
        }
    }

    if (out_dir.empty() ||
        (osm_file.empty() && bbox_str.empty() && gpx_file.empty()))
    {
        std::fprintf(stderr,
            "Usage:\n"
            "  osm-import --bbox minlat,minlon,maxlat,maxlon --out <route_dir> [options]\n"
            "  osm-import --file <file.osm> --out <route_dir> [options]\n"
            "  osm-import --gpx <track.gpx> [--hgt <dir|file.hgt>] --out <route_dir> [options]\n"
            "Options:\n"
            "  --hgt PATH       SRTM heights file or directory (elevation of tracks)\n"
            "  --overpass URL   Overpass mirror (default overpass-api.de)\n"
            "  --simplify M     polyline simplification, m (0.5)\n"
            "  --minlen M       min trajectory length, m (20)\n"
            "  --prefix NAME    trajectory name prefix (osm)\n"
            "  --service        include sidings/yard tracks\n"
            "  --name NAME      route display name\n");
        return 1;
    }

    // GPX-режим
    if (!gpx_file.empty())
    {
        return import_gpx(gpx_file, hgt_path, out_dir, route_name, options);
    }

    osm::GeoHeight geo(hgt_path);

    osm::OsmData data;
    std::string error = "";

    if (!osm_file.empty())
    {
        std::printf("Parsing OSM file: %s\n", osm_file.c_str());

        if (!osm::parse_osm_file(osm_file, data, &error))
        {
            std::fprintf(stderr, "FAILED: %s\n", error.c_str());
            return 1;
        }
    }
    else
    {
        double min_lat = 0.0;
        double min_lon = 0.0;
        double max_lat = 0.0;
        double max_lon = 0.0;

        if (std::sscanf(bbox_str.c_str(), "%lf,%lf,%lf,%lf",
                        &min_lat, &min_lon, &max_lat, &max_lon) != 4)
        {
            std::fprintf(stderr, "Bad bbox: %s (expected minlat,minlon,maxlat,maxlon)\n",
                         bbox_str.c_str());
            return 1;
        }

        std::printf("Downloading railways from Overpass: bbox %s\n", bbox_str.c_str());
        std::printf("Overpass: %s\n", overpass_url.c_str());

        std::string xml = "";
        if (!osm::download_overpass(overpass_url, min_lat, min_lon, max_lat, max_lon,
                                    xml, &error))
        {
            std::fprintf(stderr, "FAILED: %s\n", error.c_str());
            return 1;
        }

        std::printf("Downloaded %zu bytes\n", xml.size());

        if (!osm::parse_osm_xml(xml, data, &error))
        {
            std::fprintf(stderr, "FAILED: %s\n", error.c_str());
            return 1;
        }
    }

    std::printf("OSM data: %zu nodes, %zu ways\n",
                data.nodes.size(), data.ways.size());

    const auto railways = osm::filter_railways(data, include_service);
    std::printf("Rail ways: %zu (service=%s)\n",
                railways.size(), include_service ? "included" : "skipped");

    if (railways.empty())
    {
        std::fprintf(stderr, "No railways found in the source\n");
        return 1;
    }

    const auto chains = osm::build_chains(data, railways);
    std::printf("Chains: %zu\n", chains.size());

    auto trajectories = osm::chains_to_trajectories(data, chains, options);

    // Высоты SRTM: применяем к каждой точке траекторий (обратная проекция)
    if (geo.hasData())
    {
        std::printf("SRTM heights: applying\n");

        const double lat0 = 0.5 * (data.min_lat + data.max_lat);
        const double lon0 = 0.5 * (data.min_lon + data.max_lon);
        const double m_lat = 111132.0;
        const double m_lon = 111320.0 * std::cos(lat0 * 0.017453292519943295);

        for (auto& traj : trajectories)
        {
            for (auto& p : traj.points)
            {
                const double lat = lat0 + p.y / m_lat;
                const double lon = lon0 + p.x / m_lon;
                p.z = geo.height(lat, lon);
            }
        }
    }

    std::printf("Trajectories: %zu, total length %.1f km\n",
                trajectories.size(), total_length(trajectories) / 1000.0);

    const double center_lat = 0.5 * (data.min_lat + data.max_lat);
    const double center_lon = 0.5 * (data.min_lon + data.max_lon);

    if (!osm::write_route(out_dir, route_name, trajectories, &error,
                          center_lat, center_lon))
    {
        std::fprintf(stderr, "FAILED: %s\n", error.c_str());
        return 1;
    }

    std::printf("Route written to %s\n", out_dir.c_str());
    std::printf("Next steps:\n");
    std::printf("  1. Open the route in route_editor2, connect trajectories\n");
    std::printf("     (switches), generate track meshes (P key window)\n");
    std::printf("  2. Run collider-gen <route_dir> for walk-mode collisions\n");
    std::printf("  3. Add a scenario or use default placement\n");

    return 0;
}
