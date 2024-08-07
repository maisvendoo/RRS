#include    "converter.h"

#include    <iostream>
#include    <QFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(const std::string &path, zds_branch_track_data_t &branch_data)
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
    return readBranchTracksDAT(stream, branch_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(QTextStream &stream, zds_branch_track_data_t &branch_data)
{
    zds_branch_track_t branch_track;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        if (line.at(0) == ';')
            continue;

        QStringList tokens = line.split(' ');

        bool is_valid_value = false;
        int id_value = tokens[0].toInt(&is_valid_value);
        if ((!is_valid_value) || (id_value < 0) || (static_cast<size_t>(id_value) > tracks_data1.size()) || (tokens.size() < 2))
            continue;

        is_valid_value = false;
        double bias_value = tokens[1].toDouble(&is_valid_value);
        if ((!is_valid_value) || (abs(bias_value) > 100.0))
            continue;

        zds_branch_point_t branch_point;
        branch_point.main_track_id = id_value;
        branch_point.bias = bias_value;
        if (tokens.size() > 2)
            branch_point.signal_liter = tokens[2].toStdString();
        if (tokens.size() > 3)
            branch_point.signal_special = tokens[3].toStdString();

        branch_track.branch_track.push_back(branch_point);

        if (abs(bias_value - ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK) < 0.01)
        {
            branch_track.is_other_main_track = true;
        }

        if (abs(bias_value) < 0.01)
        {
            zds_branch_track_t tmp_branch_track = branch_track;
            branch_data.push_back(tmp_branch_track);

            branch_track.branch_track.clear();
            branch_track.is_other_main_track = false;
        }
    }
    return true;
}
