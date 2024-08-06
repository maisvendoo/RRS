#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSpeedsDAT(const std::string &path, zds_speeds_data_t &speeds_data)
{
    if (path.empty())
        return false;

    std::string file_path = path;

    QFile file(QString(file_path.c_str()));

    if (!file.open(QIODevice::ReadOnly))
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    QTextStream stream(&file);

    return readSpeedsDAT(stream, speeds_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSpeedsDAT(QTextStream &stream, zds_speeds_data_t &speeds_data)
{
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        QStringList tokens = line.split(' ');

        zds_speeds_t speed_point;
        speed_point.begin_track_id = static_cast<size_t>(tokens[0].toInt());
        speed_point.end_track_id = static_cast<size_t>(tokens[1].toInt());
        speed_point.limit = tokens[2].toDouble();

        speed_point.begin_trajectory_coord = tracks_data1[speed_point.begin_track_id].trajectory_coord;
        speed_point.end_trajectory_coord = tracks_data1[speed_point.end_track_id].trajectory_coord;

        speeds_data.push_back(speed_point);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeSpeeds(const std::string &filename, const zds_speeds_data_t &speeds_data)
{
    std::string path = compinePath(toNativeSeparators(routeDir), filename);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename((filename + ".bak").c_str());

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (auto it = speeds_data.begin(); it != speeds_data.end(); ++it)
    {
        stream << (*it).begin_trajectory_coord << ";"
               << (*it).limit << "\n";
    }

    file.close();
}
