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
        cur_track.trav = normalize(dvec3(cur_track.orth.x, cur_track.orth.y, 0.0));

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
            if (railway_coord_recalc_count == 0)
                railway_coord_recalc_begin = static_cast<double>(cur_track.ordinate);
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
void ZDSimConverter::splitMainTrajectory(const int &dir)
{
    if (dir > 0)
    {
        split_zds_trajectory_t split = split_zds_trajectory_t();
        split_zds_trajectory_t split_next = split_zds_trajectory_t();
        split_zds_trajectory_t split2 = split_zds_trajectory_t();
        split_zds_trajectory_t split2_next = split_zds_trajectory_t();
        bool is_add_split_next = false;
        bool is_add_split2_next = false;

        size_t id = 0;
        for (auto it = tracks_data1.begin(); it != tracks_data1.end(); ++it)
        {
            if (is_add_split_next)
                split = split_next;
            else
                split = split_zds_trajectory_t();
            split_next = split_zds_trajectory_t();
            is_add_split_next = false;

            if (is_add_split2_next)
                split2 = split2_next;
            else
                split2 = split_zds_trajectory_t();
            split2_next = split_zds_trajectory_t();
            is_add_split2_next = false;
            split2.track_id = id;

            zds_track_t track = *it;

            // Съезды между главными путями
            if (track.arrows == "2+2")
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                // Добавляем разделение и в соседний главный путь
                split2_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                is_add_split2_next = true;
            }
            if (track.arrows == "2-2")
            {
                split_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);
                is_add_split_next = true;
                // Добавляем разделение и в соседний главный путь
                split2.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);
            }
/* Съезды в однопутный участок проще искать через совпадение точек
            if (track.arrows == "1+2")
            if (track.arrows == "1-2")
            if (track.arrows == "2+1")
            if (track.arrows == "2-1")
*/
            // Стрелки на боковые пути
            if (track.arrows == "+2")
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_PLUS2);
            }
            if (track.arrows == "-2")
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_MINUS2);
            }
            if (track.arrows == "2+")
            {
                split_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS);
                is_add_split_next = true;
            }
            if (track.arrows == "2-")
            {
                split_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS);
                is_add_split_next = true;
            }

            for (auto s_it = signals_data1.begin(); s_it != signals_data1.end(); ++s_it)
            {
                if ((*s_it).track_id == id)
                {
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_FWD);
                    split.signal_fwd_type = (*s_it).type;
                    split.signal_fwd_liter = (*s_it).liter;
                    break;
                }
            }

            if (!split.split_type.empty())
            {
                split.track_id = id;
                split.point = track.begin_point;
                split.railway_coord = track.railway_coord;
                split_data1.push_back(new split_zds_trajectory_t(split));
            }
            if (!split2.split_type.empty())
            {
                split2.track_id = id;
                split_data2.push_back(new split_zds_trajectory_t(split2));
            }

            ++id;
        }
    }
    else
    {
        split_zds_trajectory_t split = split_zds_trajectory_t();
        split_zds_trajectory_t split_next = split_zds_trajectory_t();
        bool is_add_split_next = false;

        size_t id = 0;
        for (auto it = tracks_data2.begin(); it != tracks_data2.end(); ++it)
        {
            if (is_add_split_next)
                split = split_next;
            else
                split = split_zds_trajectory_t();
            split_next = split_zds_trajectory_t();
            is_add_split_next = false;

            zds_track_t track = *it;

            // Стрелки на боковые пути
            if (track.arrows == "+2")
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_PLUS2);
            }
            if (track.arrows == "-2")
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_MINUS2);
            }
            if (track.arrows == "2+")
            {
                split_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS);
                is_add_split_next = true;
            }
            if (track.arrows == "2-")
            {
                split_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS);
                is_add_split_next = true;
            }

            for (auto s_it = signals_data2.begin(); s_it != signals_data2.end(); ++s_it)
            {
                if ((*s_it).track_id + 1 == id)
                {
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_BWD);
                    split.signal_bwd_type = (*s_it).type;
                    split.signal_bwd_liter = (*s_it).liter;
                    break;
                }
            }

            bool is_split_exist = false;
            auto exist_it = split_data2.begin();
            while (exist_it != split_data2.end())
            {
                if ((*exist_it)->track_id == id)
                {
                    is_split_exist = true;
                    break;
                }

                ++exist_it;
            }

            if (is_split_exist)
            {
                (*exist_it)->point = track.begin_point;
                (*exist_it)->railway_coord = track.railway_coord;
            }

            if (!split.split_type.empty())
            {
                if (is_split_exist)
                {
                    for (auto type_it : split.split_type)
                    {
                        (*exist_it)->split_type.push_back(type_it);
                        if (type_it == split_zds_trajectory_t::SPLIT_SIGNAL_BWD)
                        {
                            (*exist_it)->signal_bwd_type = split.signal_bwd_type;
                            (*exist_it)->signal_bwd_type = split.signal_bwd_liter;
                        }
                    }
                }
                else
                {
                    split.track_id = id;
                    split.point = track.begin_point;
                    split.railway_coord = track.railway_coord;
                    split_data2.push_back(new split_zds_trajectory_t(split));
                }
            }

            ++id;
        }
    }

    // Вывод в файл, для отладки
    std::string path;
    route_connectors_t* data;
    if (dir > 0)
    {
        path = compinePath(toNativeSeparators(routeDir), "split1.conf");
        data = &split_data1;
    }
    else
    {
        path = compinePath(toNativeSeparators(routeDir), "split2.conf");
        data = &split_data2;
    }

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + ".bak").c_str()) );

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto it = data->begin(); it != data->end(); ++it)
    {
        stream << (*it)->point.x << ";"
               << (*it)->point.y << ";"
               << (*it)->point.z << ";"
               << (*it)->track_id << ";"
               << static_cast<int>(round((*it)->railway_coord)) << ";";
        for (auto type_it : (*it)->split_type)
        {
            switch (type_it)
            {
            case split_zds_trajectory_t::SPLIT_2PLUS2:
            case split_zds_trajectory_t::SPLIT_2MINUS2:
            {
                stream << "splitby2-2_" << type_it << ";";
                break;
            }
            case split_zds_trajectory_t::SPLIT_PLUS2:
            case split_zds_trajectory_t::SPLIT_MINUS2:
            case split_zds_trajectory_t::SPLIT_2PLUS:
            case split_zds_trajectory_t::SPLIT_2MINUS:
            {
                stream << "splitbyx2x_" << type_it << ";";
                break;
            }
            case split_zds_trajectory_t::SPLIT_SIGNAL_FWD:
            case split_zds_trajectory_t::SPLIT_SIGNAL_BWD:
            {
                stream << "splitsign_" << type_it << ";";
                break;
            }
            default: stream << "split_" << type_it << ";";
            }
        }
        stream << "\n";
    }

    file.close();
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
