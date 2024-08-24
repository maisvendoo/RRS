#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readStartKilometersDAT(const std::string &path,
                                   zds_start_km_data_t &waypoints)
{
    if (path.empty())
        return false;

    QString data = fileToQString(path);
    if (data.isEmpty())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    QTextStream stream(&data);
    return readStartKilometersDAT(stream, waypoints);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readStartKilometersDAT(QTextStream &stream,
                                   zds_start_km_data_t &waypoints)
{
    size_t station_id = 0;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        QStringList tokens = line.split(' ');
        if (tokens.size() < 2)
            continue;

        zds_start_km_t waypoint;
        waypoint.name = tokens[0].toStdString();
        bool is_valid_value = false;
        int id_value = tokens[1].toInt(&is_valid_value);
        if ((!is_valid_value) || (id_value < 1) || (static_cast<size_t>(id_value) > tracks_data1.size()))
            continue;

        ++station_id;
        waypoint.forward_track_id = id_value - 1;
        waypoint.forward_route_coord = tracks_data1[waypoint.forward_track_id].route_coord;
        waypoint.mean_point = tracks_data1[waypoint.forward_track_id].begin_point;
        waypoint.station_id = station_id;
        start_point_t start_point_fwd;
        start_point_fwd.name = waypoint.name + " (туда)";
        start_point_fwd.trajectory_name = tracks_data1[waypoint.forward_track_id].trajectory_name;
        start_point_fwd.direction = 1;
        start_point_fwd.trajectory_coord = tracks_data1[waypoint.forward_track_id].trajectory_coord;
        start_point_fwd.railway_coord = tracks_data1[waypoint.forward_track_id].railway_coord;
        start_point_fwd.station_id = station_id;
        start_points.push_back(new start_point_t(start_point_fwd));

        if ((!tracks_data2.empty()) && (tokens.size() > 2))
        {
            is_valid_value = false;
            id_value = tokens[2].toInt(&is_valid_value);
            if ((is_valid_value) && (id_value >= 1) && (static_cast<size_t>(id_value) <= tracks_data2.size()))
            {
                waypoint.backward_track_id = id_value - 1;
                waypoint.backward_route_coord = tracks_data2[waypoint.backward_track_id].route_coord +
                                                tracks_data2[waypoint.backward_track_id].length;
                waypoint.mean_point = waypoint.mean_point * 0.5 +
                    tracks_data2[waypoint.backward_track_id].end_point * 0.5;
                start_point_t start_point_bwd;
                start_point_bwd.name = waypoint.name + " (обратно)";
                start_point_bwd.trajectory_name = tracks_data2[waypoint.backward_track_id].trajectory_name;
                start_point_bwd.direction = -1;
                start_point_bwd.trajectory_coord = tracks_data2[waypoint.backward_track_id].trajectory_coord +
                                                   tracks_data2[waypoint.backward_track_id].length;
                start_point_bwd.railway_coord = tracks_data2[waypoint.backward_track_id].railway_coord_end;
                start_point_bwd.station_id = station_id;
                start_points.push_back(new start_point_t(start_point_bwd));
            }
        }

        waypoints.push_back(waypoint);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::findStartPointsBySignals(const route_connectors_t &connectors)
{
    // Точка за 15 метров до светофора
    double add_distance_before_signal = 15.0;

    double min_distance;
    zds_start_km_t* nearest_station;
    for (auto connector : connectors)
    {
        // Светофор назад
        min_distance = 1e10;
        nearest_station = nullptr;
        std::string bwd_liter = connector->signal_bwd_liter;
        if (!bwd_liter.empty())
        {
            dvec3 signal_point = connector->point;

            bool is_digits_only = true;
            for (auto c : bwd_liter)
            {
                is_digits_only &= std::isdigit(c);
            }

            if (is_digits_only)
            {
                // Если светофор без букв в литере, считаем это перегоном
                //TODO
            }
            else
            {
                // Остальные светофоры привязываем к ближайшей станции
                for (auto station = start_km_data.begin(); station != start_km_data.end(); ++station)
                {
                    double distance = length(station->mean_point - signal_point);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        nearest_station = &(*station);
                    }
                }
                if (nearest_station != nullptr)
                {
                    start_point_t start_point;
                    start_point.name = nearest_station->name + " (" + bwd_liter + ")";
                    start_point.trajectory_name = connector->fwd_main_traj;
                    start_point.direction = -1;
                    start_point.trajectory_coord = add_distance_before_signal;
                    start_point.railway_coord = connector->railway_coord + add_distance_before_signal;
                    start_point.station_id = nearest_station->station_id;
                    start_points.push_back(new start_point_t(start_point));
                }
            }
        }
        // Светофор вперёд
        min_distance = 1e10;
        nearest_station = nullptr;
        std::string fwd_liter = connector->signal_fwd_liter;
        if (!fwd_liter.empty())
        {
            dvec3 signal_point = connector->point;

            bool is_digits_only = true;
            for (auto c : fwd_liter)
            {
                is_digits_only &= std::isdigit(c);
            }

            if (is_digits_only)
            {
                // Если светофор без букв в литере, считаем это перегоном
                //TODO
            }
            else
            {
                // Остальные светофоры привязываем к ближайшей станции
                for (auto station = start_km_data.begin(); station != start_km_data.end(); ++station)
                {
                    double distance = length(station->mean_point - signal_point);
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        nearest_station = &(*station);
                    }
                }
                if (nearest_station != nullptr)
                {
                    start_point_t start_point;
                    start_point.name = nearest_station->name + " (" + fwd_liter + ")";
                    start_point.trajectory_name = connector->bwd_main_traj;
                    start_point.direction = 1;
                    start_point.trajectory_coord = connector->length_bwd_traj - add_distance_before_signal;
                    start_point.railway_coord = connector->railway_coord - add_distance_before_signal;
                    start_point.station_id = nearest_station->station_id;
                    start_points.push_back(new start_point_t(start_point));
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeStationsOld(const std::string &filename, const zds_start_km_data_t &waypoints)
{
    std::string path = compinePath(toNativeSeparators(routeDir), filename);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (auto it = waypoints.begin(); it != waypoints.end(); ++it)
    {
        double min_coord = std::min((*it).forward_route_coord,(*it).backward_route_coord);
        double max_coord = std::max((*it).forward_route_coord,(*it).backward_route_coord);
        stream << min_coord << ";"
               << max_coord << ";"
               << (*it).name.c_str() << "\n";
    }

    file.close();
}
