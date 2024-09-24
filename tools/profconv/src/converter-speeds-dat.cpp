#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSpeedsDAT(const std::string &path, zds_speeds_data_t &speeds_data, const int &dir)
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
    return readSpeedsDAT(stream, speeds_data, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readSpeedsDAT(QTextStream &stream, zds_speeds_data_t &speeds_data, const int &dir)
{
    zds_trajectory_data_t* td = dir > 0 ? &tracks_data1 : &tracks_data2;
    int last_id = -1;
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
        if ((!is_valid_value) || (begin_id_value < 0) || (static_cast<size_t>(begin_id_value) > (*td).size()))
            continue;
        if (begin_id_value == 0)
            begin_id_value = 1;

        is_valid_value = false;
        int end_id_value = tokens[1].toInt(&is_valid_value);
        if ((!is_valid_value) || (end_id_value < 0) || (static_cast<size_t>(end_id_value) > (*td).size()))
            continue;
        if (end_id_value == 0)
            end_id_value = 1;


        is_valid_value = false;
        double limit_value = tokens[2].toDouble(&is_valid_value);
        if ((!is_valid_value) || (limit_value < 1.0))
            continue;
        // В некоторых маршрутах встречается ограничение скорости
        // меньше на 1-3 км/ч для обхода механик ZDS,
        // поэтому округляем вверх до кратного пяти
        limit_value = 5.0 * ceil(limit_value / 5.0);

        zds_speeds_t speed_point;
        speed_point.begin_track_id = std::min(begin_id_value, end_id_value) - 1;
        speed_point.end_track_id = std::max(begin_id_value, end_id_value) - 1;
        speed_point.limit = limit_value;

        // Проверяем что ZDS треки перечислены по возрастанию/убыванию
        if (last_id == -1)
        {
            if (dir > 0)
                last_id = speed_point.end_track_id;
            else
                last_id = speed_point.begin_track_id;
        }
        else
        {
            if (dir > 0)
            {
                if (speed_point.end_track_id <= last_id + 1)
                {
                    continue;
                }
                if (speed_point.begin_track_id <= last_id)
                {
                    speed_point.begin_track_id = last_id + 1;
                }
                last_id = speed_point.end_track_id;
            }
            else
            {
                if (speed_point.begin_track_id >= last_id - 1)
                {
                    continue;
                }
                if (speed_point.end_track_id >= last_id)
                {
                    speed_point.end_track_id = last_id - 1;
                }
                last_id = speed_point.begin_track_id;
            }
        }

        speed_point.begin_route_coord = (*td)[speed_point.begin_track_id].route_coord;
        speed_point.end_route_coord = (*td)[speed_point.end_track_id].route_coord +
                                           (*td)[speed_point.end_track_id].length;

        speed_point.begin_railway_coord = (*td)[speed_point.begin_track_id].railway_coord;
        speed_point.end_railway_coord = (*td)[speed_point.end_track_id].railway_coord_end;

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
bool ZDSimConverter::createSpeedMap()
{
    // Карта скоростей для пути "туда"
    if (speeds_data1.empty())
        return false;

    // Готовим массив данных под карты туда и обратно по участкам пикетажа
    // и карту на все съезды и боковые пути
    size_t size = railway_coord_sections1 + railway_coord_sections2 + 1;
    speedmap_data.clear();
    for (size_t i = 0; i < size; ++i)
    {
        speedmap_data.push_back(new speedmap_t());
    }

    // Пишем список траекторий по главному пути
    for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
    {
        int railway_coord_section = (*traj)->railway_coord_section;
        int speedmap_id = railway_coord_section;

        speedmap_data[speedmap_id]->name_prefix = "route1";
        speedmap_data[speedmap_id]->trajectories_names.push_back((*traj)->name);
    }

    // Пишем список ограничений скорости
    for (auto zds_speed : speeds_data1)
    {
        // Ограничение на интервале пикетажа
        speed_element_t speed_el = speed_element_t();
        speed_el.limit = zds_speed.limit;
        speed_el.railway_coord_begin = static_cast<int>(round(zds_speed.begin_railway_coord));
        speed_el.railway_coord_end = static_cast<int>(round(zds_speed.end_railway_coord));

        size_t id_begin = zds_speed.begin_track_id;
        size_t id_end = zds_speed.end_track_id;
        int railway_coord_section = tracks_data1[id_begin].railway_coord_section;
        int speedmap_id = railway_coord_section;
        for (size_t id = id_begin + 1; id <= id_end; ++id)
        {
            // Если начался новый участок жд-пикетажа,
            // завершаем интервал с ограничением скорости и начинаем новый
            if (tracks_data1[id].railway_coord_section != railway_coord_section)
            {
                speed_element_t speed_el_end_here = speed_el;
                speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data1[id - 1].railway_coord_end));

                speedmap_t *speedmap = speedmap_data[speedmap_id];
                speedmap->speedmap_elements.push_back(speed_el_end_here);

                speed_el.railway_coord_begin = static_cast<int>(round(tracks_data1[id].railway_coord));

                railway_coord_section = tracks_data1[id].railway_coord_section;
                speedmap_id = railway_coord_section;
            }
        }
        speedmap_data[speedmap_id]->speedmap_elements.push_back(speed_el);
    }

    // Пишем список всех траекторий боковых путей и съездов
    int speedmap_id = railway_coord_sections1;
    speedmap_data[speedmap_id]->name_prefix = "all_branch_tracks";
    if (!branch_track_data1.empty())
    {
        for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
        {
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                speedmap_data[speedmap_id]->trajectories_names.push_back((*traj)->name);
            }
        }
    }
    if (!branch_track_data2.empty())
    {
        for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
        {
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                speedmap_data[speedmap_id]->trajectories_names.push_back((*traj)->name);
            }
        }
    }
    if (!branch_2minus2_data.empty())
    {
        for (auto it = branch_2minus2_data.begin(); it != branch_2minus2_data.end(); ++it)
        {
            speedmap_data[speedmap_id]->trajectories_names.push_back((*it)->trajectory.name);
        }
    }
    if (!branch_2plus2_data.empty())
    {
        for (auto it = branch_2plus2_data.begin(); it != branch_2plus2_data.end(); ++it)
        {
            speedmap_data[speedmap_id]->trajectories_names.push_back((*it)->trajectory.name);
        }
    }
    // Ограничение 40 км/ч на любом интервале пикетажа для всех боковых путей
    if (!speedmap_data[speedmap_id]->trajectories_names.empty())
    {
        speed_element_t speed_el = speed_element_t();
        speed_el.limit = 40;
        speedmap_data[speedmap_id]->speedmap_elements.push_back(speed_el);
    }

    // Карта скоростей для пути "обратно"
    if (trajectories2.empty() || speeds_data2.empty())
        return true;

    // Пишем список траекторий по главному пути
    for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
    {
        int railway_coord_section = (*traj)->railway_coord_section;
        int speedmap_id = railway_coord_sections1 + 1 + railway_coord_section;

        speedmap_data[speedmap_id]->name_prefix = "route2";
        speedmap_data[speedmap_id]->trajectories_names.push_back((*traj)->name);
    }

    // Пишем список ограничений скорости
    for (auto zds_speed : speeds_data2)
    {
        // Ограничение на интервале пикетажа
        speed_element_t speed_el = speed_element_t();
        speed_el.limit = zds_speed.limit;
        speed_el.railway_coord_begin = static_cast<int>(round(zds_speed.begin_railway_coord));
        speed_el.railway_coord_end = static_cast<int>(round(zds_speed.end_railway_coord));

        size_t id_begin = zds_speed.begin_track_id;
        size_t id_end = zds_speed.end_track_id;

        int railway_coord_section = tracks_data2[id_begin].railway_coord_section;
        int speedmap_id = railway_coord_sections1 + 1 + railway_coord_section;
        bool empty_section = speedmap_data[speedmap_id]->trajectories_names.empty();

        bool was_1_track = (tracks_data2[id_begin].id_at_track1 > -1);

        for (size_t id = id_begin + 1; id <= id_end; ++id)
        {
            if (tracks_data2[id].id_at_track1 > -1)
            {
                if (!was_1_track)
                {
                    // Начало однопутного участка,
                    // завершаем интервал с ограничением скорости
                    if (!empty_section)
                    {
                        speed_element_t speed_el_end_here = speed_el;
                        speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data2[id - 1].railway_coord_end));

                        if (speed_el_end_here.railway_coord_begin != speed_el_end_here.railway_coord_end)
                        {
                            speedmap_data[speedmap_id]->speedmap_elements.push_back(speed_el_end_here);
                        }
                    }
                }

                // Однопутный участок
                was_1_track = true;
            }
            else
            {
                if (was_1_track)
                {
                    // Конец однопутного участка,
                    // начинаем новый интервал ограничения скорости
                    speed_el.railway_coord_begin = static_cast<int>(round(tracks_data2[id].railway_coord));
                }

                // Двухпутный участок
                was_1_track = false;
            }

            // Если начался новый участок жд-пикетажа,
            // завершаем интервал с ограничением скорости и начинаем новый
            if (tracks_data2[id].railway_coord_section != railway_coord_section)
            {
                if (!(empty_section || was_1_track))
                {
                    speed_element_t speed_el_end_here = speed_el;
                    speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data2[id - 1].railway_coord_end));

                    if (speed_el_end_here.railway_coord_begin != speed_el_end_here.railway_coord_end)
                    {
                        speedmap_data[speedmap_id]->speedmap_elements.push_back(speed_el_end_here);
                    }
                }

                speed_el.railway_coord_begin = static_cast<int>(round(tracks_data2[id].railway_coord));

                railway_coord_section = tracks_data2[id].railway_coord_section;
                speedmap_id = railway_coord_sections1 + 1 + railway_coord_section;
                empty_section = speedmap_data[speedmap_id]->trajectories_names.empty();
            }
        }

        if ((!(empty_section || was_1_track)) && (speed_el.railway_coord_begin != speed_el.railway_coord_end))
            speedmap_data[speedmap_id]->speedmap_elements.push_back(speed_el);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeOldSpeeds(const std::string &filename, const zds_speeds_data_t &speeds_data)
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

    for (auto it = speeds_data.begin(); it != speeds_data.end(); ++it)
    {
        stream << (*it).begin_route_coord << ";"
               << (*it).limit << "\n";
    }

    file.close();
}
