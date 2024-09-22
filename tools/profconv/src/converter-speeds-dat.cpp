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

    // Пишем список траекторий по главному пути
    int railway_coord_section = 0;
    speedmap_t *speedmap = new speedmap_t();
    speedmap->name_prefix = "route1";
    for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
    {
        // Если начался новый участок жд-пикетажа,
        // заканчиваем список траекторий и начинаем новый
        if ((*traj)->railway_coord_section != railway_coord_section)
        {
            railway_coord_section = (*traj)->railway_coord_section;
            speedmap_data.push_back(speedmap);
            speedmap = new speedmap_t();
            speedmap->name_prefix = "route1";
        }
        speedmap->trajectories_names.push_back((*traj)->name);
    }
    speedmap_data.push_back(speedmap);

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
        railway_coord_section = tracks_data1[id_begin].railway_coord_section;
        for (size_t id = id_begin + 1; id <= id_end; ++id)
        {
            // Если начался новый участок жд-пикетажа,
            // завершаем интервал с ограничением скорости и начинаем новый
            if (tracks_data1[id].railway_coord_section != railway_coord_section)
            {
                speed_element_t speed_el_end_here = speed_el;
                speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data1[id - 1].railway_coord_end));
                speedmap_data[railway_coord_section]->speedmap_elements.push_back(speed_el_end_here);

                speed_el.railway_coord_begin = static_cast<int>(round(tracks_data1[id].railway_coord));

                railway_coord_section = tracks_data1[id].railway_coord_section;
            }
        }
        speedmap_data[railway_coord_section]->speedmap_elements.push_back(speed_el);
    }

    // Пишем список всех траекторий боковых путей и съездов
    speedmap_t *branch_speedmap = new speedmap_t();
    branch_speedmap->name_prefix = "all_branch_tracks";
    if (!branch_track_data1.empty())
    {
        for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
        {
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                branch_speedmap->trajectories_names.push_back((*traj)->name);
            }
        }
    }
    if (!branch_track_data2.empty())
    {
        for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
        {
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                branch_speedmap->trajectories_names.push_back((*traj)->name);
            }
        }
    }
    if (!branch_2minus2_data.empty())
    {
        for (auto it = branch_2minus2_data.begin(); it != branch_2minus2_data.end(); ++it)
        {
            branch_speedmap->trajectories_names.push_back((*it)->trajectory.name);
        }
    }
    if (!branch_2plus2_data.empty())
    {
        for (auto it = branch_2plus2_data.begin(); it != branch_2plus2_data.end(); ++it)
        {
            branch_speedmap->trajectories_names.push_back((*it)->trajectory.name);
        }
    }
    // Ограничение 40 км/ч на любом интервале пикетажа для всех боковых путей
    if (!branch_speedmap->trajectories_names.empty())
    {
        speed_element_t speed_el = speed_element_t();
        speed_el.limit = 40;
        branch_speedmap->speedmap_elements.push_back(speed_el);
        speedmap_data.push_back(branch_speedmap);
    }

    // Карта скоростей для пути "обратно"
    if (trajectories2.empty() || speeds_data2.empty())
        return false;

    // Пишем список траекторий по главному пути
    speedmap = new speedmap_t();
    speedmap->name_prefix = "route2";
    std::vector<int> railway_coord_sections = {0};
    int data1_size = speedmap_data.size();
    for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
    {
        if ((*traj)->railway_coord_section != railway_coord_sections.back())
        {
            // Если начался новый участок жд-пикетажа,
            // заканчиваем список траекторий и начинаем новый
            railway_coord_sections.push_back((*traj)->railway_coord_section);
            speedmap_data.push_back(speedmap);
            speedmap = new speedmap_t();
            speedmap->name_prefix = "route2";
        }
        speedmap->trajectories_names.push_back((*traj)->name);
    }
    speedmap_data.push_back(speedmap);

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
        railway_coord_section = tracks_data2[id_begin].railway_coord_section;
        bool was_1_track = (tracks_data2[id_begin].id_at_track1 != -1);
        for (size_t id = id_begin + 1; id <= id_end; ++id)
        {
            if (tracks_data2[id].id_at_track1 != -1)
            {
                if ((!was_1_track) && (tracks_data2[id].id_at_track1 != 0))
                {
                    // Начало однопутного участка
                    // Завершаем интервал с ограничением скорости
                    speed_element_t speed_el_end_here = speed_el;
                    speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data2[id - 1].railway_coord_end));
                    speedmap_data[data1_size + railway_coord_section]->speedmap_elements.push_back(speed_el_end_here);

                }
                if ((id + 1 == id_end) || (tracks_data2[id + 1].id_at_track1 != -1))
                {
                    // Однопутный участок
                    was_1_track = true;
                }
                else
                {
                    // Конец однопутного участка
                    // начинаем новый интервал с ограничением скорости
                    if (was_1_track)
                    {
                        was_1_track = false;
                        speed_el.railway_coord_begin = static_cast<int>(round(tracks_data2[id].railway_coord));
                    }
                }
            }

            // Если начался новый участок жд-пикетажа,
            // завершаем интервал с ограничением скорости и начинаем новый
            if (tracks_data2[id].railway_coord_section != railway_coord_section)
            {
                speed_element_t speed_el_end_here = speed_el;
                speed_el_end_here.railway_coord_end = static_cast<int>(round(tracks_data2[id - 1].railway_coord_end));
                speedmap_data[data1_size + railway_coord_section]->speedmap_elements.push_back(speed_el_end_here);

                speed_el.railway_coord_begin = static_cast<int>(round(tracks_data2[id].railway_coord));

                railway_coord_section = tracks_data2[id].railway_coord_section;
            }
        }
        speedmap_data[data1_size + railway_coord_section]->speedmap_elements.push_back(speed_el);
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
