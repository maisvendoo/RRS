#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSvetoforDAT(const std::string &path, zds_signals_data_t &signals_data, const int &dir)
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
    return readSvetoforDAT(stream, signals_data, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSvetoforDAT(QTextStream &stream, zds_signals_data_t &signals_data, const int &dir)
{
    zds_trajectory_data_t* td = dir > 0 ? &tracks_data1 : &tracks_data2;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        QStringList tokens = line.split('\t');

        bool is_valid_value = false;
        int id_value = tokens[0].toInt(&is_valid_value);
        if ((!is_valid_value) || (id_value < 1) || (static_cast<size_t>(id_value) > (*td).size()) || (tokens.size() < 2))
            continue;

        zds_signals_t signal;
        signal.track_id = id_value - 1;
        signal.type = tokens[1].toStdString();
        if (tokens.size() > 2)
            signal.liter = tokens[2].toStdString();
        if (tokens.size() > 3)
            signal.special = tokens[3].toStdString();

        signal.route_coord = (*td)[signal.track_id].route_coord;

        signals_data.push_back(signal);
    }

    std::sort(signals_data.begin(),
              signals_data.end(),
              zds_signals_t::compare_by_track_id);

    return true;
}
