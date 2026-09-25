//------------------------------------------------------------------------------
//
//      osm-import: запись маршрута RRS
//
//------------------------------------------------------------------------------

#include "route-writer.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <set>

namespace osm
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<Trajectory> chains_to_trajectories(const OsmData& data,
                                                const std::vector<Chain>& chains,
                                                const WriteOptions& options)
{
    // Начало координат - центр bbox
    Projector projector(0.5 * (data.min_lat + data.max_lat),
                        0.5 * (data.min_lon + data.max_lon));

    std::vector<Trajectory> result;
    std::set<std::string> used_names;

    int index = 1;

    for (const Chain& chain : chains)
    {
        std::vector<LocalPoint> points;
        points.reserve(chain.node_ids.size());

        for (const std::uint64_t node_id : chain.node_ids)
        {
            const auto it = data.nodes.find(node_id);
            if (it == data.nodes.end())
            {
                continue;
            }

            LocalPoint p = projector.toLocal(it->second.lat, it->second.lon);
            p.z = options.track_height_m;
            points.push_back(p);
        }

        if (points.size() < 2)
        {
            continue;
        }

        // Убираем подряд дублирующиеся точки
        std::vector<LocalPoint> filtered;
        filtered.reserve(points.size());

        for (const LocalPoint& p : points)
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

        if (filtered.size() < 2)
        {
            continue;
        }

        if (options.simplify_epsilon_m > 0.0)
        {
            filtered = simplify_polyline(filtered, options.simplify_epsilon_m);
        }

        Trajectory traj;
        traj.points = filtered;
        traj.length_m = polyline_length(filtered);

        if (traj.length_m < options.min_trajectory_length_m)
        {
            continue;
        }

        // Уникальное имя: osm_0001, osm_0002...
        while (true)
        {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%s_%04d",
                          options.name_prefix.c_str(), index);
            traj.name = buf;
            ++index;

            if (used_names.count(traj.name) == 0)
            {
                used_names.insert(traj.name);
                break;
            }
        }

        result.push_back(std::move(traj));
    }

    // Сортировка по длине: главные пути первыми
    std::sort(result.begin(), result.end(),
              [](const Trajectory& a, const Trajectory& b)
              {
                  return a.length_m > b.length_m;
              });

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void write_line(QTextStream& stream, const QString& line)
{
    stream << line << "\n";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool write_route(const std::string& route_dir,
                 const std::string& route_name,
                 const std::vector<Trajectory>& trajectories,
                 std::string* error,
                 double center_lat, double center_lon)
{
    QDir dir(QString::fromStdString(route_dir));

    if (!dir.mkpath("."))
    {
        if (error != nullptr)
        {
            *error = "Can't create route directory: " + route_dir;
        }
        return false;
    }

    dir.mkpath("topology");
    dir.mkpath("topology/trajectories");

    // description.xml
    {
        QFile file(dir.filePath("description.xml"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr) *error = "Can't write description.xml";
            return false;
        }
        // Кириллица: пишем байты UTF-8 явно (QTextStream в файл по
        // умолчанию не UTF-8). Плюс гео-привязка: конвенция движка
        // X=восток, Y=север, Z=вверх; центр bbox = точка (0,0);
        // Sun/погода читают Latitude/Longitude из description.xml
        QString text;
        QTextStream ts(&text);
        write_line(ts, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        write_line(ts, "<Config>");
        write_line(ts, QString("\t<Name>%1</Name>").arg(QString::fromStdString(route_name)));
        write_line(ts, QString("\t<Latitude>%1</Latitude>").arg(center_lat, 0, 'f', 6));
        write_line(ts, QString("\t<Longitude>%1</Longitude>").arg(center_lon, 0, 'f', 6));
        write_line(ts, "\t<Description>Сгенерировано osm-import из OpenStreetMap</Description>");
        write_line(ts, "</Config>");
        file.write(text.toUtf8());
        file.close();
    }

    // route-type
    {
        QFile file(dir.filePath("route-type"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr) *error = "Can't write route-type";
            return false;
        }
        QTextStream s(&file);
        write_line(s, "main");
        file.close();
    }

    // objects.ref - пустой список моделей
    {
        QFile file(dir.filePath("objects.ref"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr) *error = "Can't write objects.ref";
            return false;
        }
        QTextStream s(&file);
        write_line(s, "# Сгенерировано osm-import: модели объектов не заданы");
        write_line(s, "# Формат: <label> <путь к модели gltf от корня маршрута>");
        file.close();
    }

    // route1.map - пустая карта объектов
    {
        QFile file(dir.filePath("route1.map"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr) *error = "Can't write route1.map";
            return false;
        }
        QTextStream s(&file);
        write_line(s, "# Сгенерировано osm-import: объекты не расставлены");
        file.close();
    }

    // topology/topology.xml - пустая конфигурация соединений
    {
        QFile file(dir.filePath("topology/topology.xml"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr) *error = "Can't write topology.xml";
            return false;
        }
        QTextStream s(&file);
        write_line(s, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        write_line(s, "<Config>");
        write_line(s, "\t<!-- Стрелки/соединения траекторий не заданы: -->");
        write_line(s, "\t<!-- добавьте в редакторе маршрутов (route_editor2) -->");
        write_line(s, "</Config>");
        file.close();
    }

    // topology/trajectories/*.traj
    // Формат строки: X Y Z railway_coord (пикетаж в метрах от начала)
    for (const Trajectory& traj : trajectories)
    {
        QFile file(dir.filePath(QString("topology/trajectories/%1.traj")
                                    .arg(QString::fromStdString(traj.name))));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if (error != nullptr)
            {
                *error = "Can't write trajectory " + traj.name;
            }
            return false;
        }

        QTextStream s(&file);

        double railway_coord = 0.0;

        for (std::size_t i = 0; i < traj.points.size(); ++i)
        {
            if (i > 0)
            {
                const double dx = traj.points[i].x - traj.points[i - 1].x;
                const double dy = traj.points[i].y - traj.points[i - 1].y;
                railway_coord += std::sqrt(dx * dx + dy * dy);
            }

            s << QString("%1 %2 %3 %4\n")
                     .arg(traj.points[i].x, 0, 'f', 3)
                     .arg(traj.points[i].y, 0, 'f', 3)
                     .arg(traj.points[i].z, 0, 'f', 3)
                     .arg(railway_coord, 0, 'f', 3);
        }

        file.close();
    }

    return true;
}

} // namespace osm
