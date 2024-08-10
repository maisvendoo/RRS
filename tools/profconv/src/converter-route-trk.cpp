#include    "converter.h"

#include    <iostream>
#include    <sstream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteTRK(const std::string &path,
                                  zds_trajectory_data_t &track_data)
{
    if (path.empty())
        return false;

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return readRouteTRK(stream, track_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteTRK(std::ifstream &stream,
                                  zds_trajectory_data_t &track_data)
{
    std::vector<zds_track_t> tmp_data;

    while (!stream.eof())
    {
        std::string line = getLine(stream);

        std::istringstream ss(line);

        zds_track_t track;

        ss >> track.begin_point.x
            >> track.begin_point.y
            >> track.begin_point.z
            >> track.end_point.x
            >> track.end_point.y
            >> track.end_point.z
            >> track.prev_uid
            >> track.next_uid
            >> track.arrows
            >> track.voltage
            >> track.ordinate;

        tmp_data.push_back(track);
    }

//    track_data.push_back(*tmp_data.begin());

    auto it = tmp_data.begin();
    double trajectory_length = 0.0;

    double coord_increase_direction = 1.0;
    double trajectory_recalc_length = 0.0;
    double railway_coord_recalc_begin = 0.0;
    size_t railway_coord_recalc_count = 0;

    bool not_end = true;
    while ( not_end )
    {
        zds_track_t cur_track = *it;
        if (cur_track.next_uid != -2)
        {
            zds_track_t next_track = tmp_data.at(static_cast<size_t>(cur_track.next_uid - 1));
            cur_track.end_point = next_track.begin_point;
            cur_track.railway_coord_end = static_cast<double>(next_track.ordinate);
            *it = next_track;
        }
        else
        {
            not_end = false;
        }

        dvec3 dir_vector = cur_track.end_point - cur_track.begin_point;
        cur_track.length = length(dir_vector);
        cur_track.orth = dir_vector / cur_track.length;
        cur_track.right = normalize(cross(cur_track.orth, dvec3(0.0, 0.0, 1.0)));
        cur_track.up = normalize(cross(cur_track.right, cur_track.orth));

        cur_track.trajectory_coord = trajectory_length;
        trajectory_length += cur_track.length;

        if (not_end)
        {
            // Сохраняем координату в начале трека, если не дробные
            if (railway_coord_recalc_count == 0)
                railway_coord_recalc_begin = static_cast<double>(cur_track.ordinate);

            // Записываем координату в начале трека
            cur_track.railway_coord = railway_coord_recalc_begin + trajectory_recalc_length;

            // В дробных треках ZDS записана одинаковая координата для всех подтреков
            if (abs(railway_coord_recalc_begin - cur_track.railway_coord_end) < 0.01)
            {
                // Считаем количество и суммарную длину подтреков, пересчитываем координату
                railway_coord_recalc_count++;
                trajectory_recalc_length += cur_track.length * coord_increase_direction;
                // Записываем пересчитанную координату конца дробного подтрека
                cur_track.railway_coord_end = railway_coord_recalc_begin + trajectory_recalc_length;
            }
            else
            {
                // Вне дробных подтреков сбрасываем счётчик
                railway_coord_recalc_count = 0;
                trajectory_recalc_length = 0.0;
                // И определяем направление увеличения координаты
                ((cur_track.railway_coord_end - railway_coord_recalc_begin) > 0) ?
                    coord_increase_direction = 1.0 :
                    coord_increase_direction = -1.0;
            }
        }
        else
        {
            cur_track.railway_coord = railway_coord_recalc_begin + trajectory_recalc_length;
            cur_track.railway_coord_end = cur_track.railway_coord + cur_track.length * coord_increase_direction;
        }

        cur_track.inclination = cur_track.orth.z * 1000.0;
        cur_track.curvature = 0.0;

        track_data.push_back(cur_track);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeProfileData(const zds_trajectory_data_t &tracks_data,
                                      const std::string &file_name)
{
    std::string path = compinePath(toNativeSeparators(routeDir), file_name);
    std::ofstream stream(path.c_str(), std::ios::out);

    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        zds_track_t track = *it;

        stream << track.trajectory_coord / 1000.0f << " "
               << track.orth.z * 1000.0f << " "
               << track.curvature << std::endl;
    }

    stream.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeMainTrajectory(const std::string &filename, const zds_trajectory_data_t &tracks_data)
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
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    size_t i = 0;
    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        ++i;
        stream << (*it).begin_point.x << ";"
               << (*it).begin_point.y << ";"
               << (*it).begin_point.z << ";"
               << static_cast<int>(round((*it).railway_coord)) << ";"
               << (*it).trajectory_coord << "\n";
    }
    stream << tracks_data.back().end_point.x << ";"
           << tracks_data.back().end_point.y << ";"
           << tracks_data.back().end_point.z << ";"
           << static_cast<int>(round(tracks_data.back().railway_coord_end)) << ";"
           << tracks_data.back().trajectory_coord + tracks_data.back().length << "\n";

    file.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::createPowerLine(const zds_trajectory_data_t &tracks_data,
                                     std::vector<power_line_element_t> &power_line)
{
    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        zds_track_t track = *it;

        power_line_element_t p_line_emem;

        p_line_emem.railway_coord = track.trajectory_coord;
        p_line_emem.voltage = static_cast<float>(track.voltage);

        switch (track.voltage)
        {
        case 3:
            p_line_emem.current_kind = DC_CURRENT;
            break;

        case 25:
            p_line_emem.current_kind = AC_CURRENT;
            break;

        default:
            p_line_emem.current_kind = NOT_DEFINED_CURRENT;
            break;
        }

        power_line.push_back(p_line_emem);
    }
}
