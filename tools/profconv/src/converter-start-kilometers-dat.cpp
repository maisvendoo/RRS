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

        zds_start_km_t waypoint;
        waypoint.name = tokens[0].toStdString();

        waypoint.forward_track_id = static_cast<size_t>(tokens[1].toInt());
        waypoint.forward_trajectory_coord = tracks_data1[waypoint.forward_track_id].trajectory_coord;
        waypoint.forward_railway_coord = tracks_data1[waypoint.forward_track_id].railway_coord;

        waypoint.backward_track_id = static_cast<size_t>(tokens[2].toInt());
        waypoint.backward_trajectory_coord = tracks_data2[waypoint.backward_track_id].trajectory_coord;
        waypoint.backward_railway_coord = tracks_data2[waypoint.backward_track_id].railway_coord;

        waypoints.push_back(waypoint);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeWaypoints(const std::string &filename,
                                    const zds_start_km_data_t &waypoints)
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
        stream << (*it).name.c_str()
               << " " << (*it).forward_trajectory_coord
               << " " << (*it).backward_trajectory_coord
               << "\n";
    }

    file.close();
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
        double min_coord = std::min((*it).forward_trajectory_coord,(*it).backward_trajectory_coord);
        double max_coord = std::max((*it).forward_trajectory_coord,(*it).backward_trajectory_coord);
        stream << min_coord << ";"
               << max_coord << ";"
               << (*it).name.c_str() << "\n";
    }

    file.close();
}
