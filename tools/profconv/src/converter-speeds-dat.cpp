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

    QString data = fileToQString(path);
    if (data.isEmpty())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    QTextStream stream(&data);
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

        QStringList tokens = line.split('\t');

        if (tokens.size() < 3)
            continue;

        bool is_valid_value = false;
        int begin_id_value = tokens[0].toInt(&is_valid_value);
        if ((!is_valid_value) || (begin_id_value < 0) || (static_cast<size_t>(begin_id_value) > tracks_data1.size()))
            continue;

        is_valid_value = false;
        int end_id_value = tokens[1].toInt(&is_valid_value);
        if ((!is_valid_value) || (end_id_value < 0) || (static_cast<size_t>(end_id_value) > tracks_data1.size()))
            continue;

        is_valid_value = false;
        double limit_value = tokens[2].toDouble(&is_valid_value);
        if ((!is_valid_value) || (limit_value < 0.0))
            continue;

        zds_speeds_t speed_point;
        speed_point.begin_track_id = std::min(begin_id_value, end_id_value);
        speed_point.end_track_id = std::max(begin_id_value, end_id_value);
        speed_point.limit = limit_value;

        speed_point.begin_trajectory_coord = tracks_data1[speed_point.begin_track_id].trajectory_coord;
        speed_point.end_trajectory_coord = tracks_data1[speed_point.end_track_id].trajectory_coord +
                                           tracks_data1[speed_point.end_track_id].length;

        speeds_data.push_back(speed_point);
    }

    std::sort(speeds_data.begin(),
              speeds_data.end(),
              zds_speeds_t::compare_by_track_id);

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
        file_old.rename( QString((path + ".bak").c_str()) );

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
