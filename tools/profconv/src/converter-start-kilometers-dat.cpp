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

        waypoint.forward_track_id = id_value - 1;
        waypoint.forward_route_coord = tracks_data1[waypoint.forward_track_id].route_coord;
        start_point_t start_point_fwd;
        start_point_fwd.name = waypoint.name;
        start_point_fwd.trajectory_name = tracks_data1[waypoint.forward_track_id].trajectory_name;
        start_point_fwd.direction = 1;
        start_point_fwd.trajectory_coord = tracks_data1[waypoint.forward_track_id].trajectory_coord;
        start_point_fwd.railway_coord = tracks_data1[waypoint.forward_track_id].railway_coord;
        waypoint.start_points.push_back(start_point_fwd);

        if ((!tracks_data2.empty()) && (tokens.size() > 2))
        {
            is_valid_value = false;
            id_value = tokens[2].toInt(&is_valid_value);
            if ((is_valid_value) && (id_value >= 1) && (static_cast<size_t>(id_value) <= tracks_data2.size()))
            {
                waypoint.backward_track_id = id_value - 1;
                waypoint.backward_route_coord = tracks_data2[waypoint.backward_track_id].route_coord +
                                                tracks_data2[waypoint.backward_track_id].length;
                start_point_t start_point_bwd;
                start_point_bwd.name = waypoint.name;
                start_point_bwd.trajectory_name = tracks_data2[waypoint.backward_track_id].trajectory_name;
                start_point_bwd.direction = -1;
                start_point_bwd.trajectory_coord = tracks_data2[waypoint.backward_track_id].trajectory_coord +
                                                   tracks_data2[waypoint.backward_track_id].length;
                start_point_bwd.railway_coord = tracks_data2[waypoint.backward_track_id].railway_coord_end;
                waypoint.start_points.push_back(start_point_bwd);
            }
        }

        waypoints.push_back(waypoint);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeStations(const std::string &filename, const zds_start_km_data_t &waypoints)
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
