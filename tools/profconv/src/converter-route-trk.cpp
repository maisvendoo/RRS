#include    "converter.h"

#include    <iostream>
#include    <sstream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteTRK(const std::string &path,
                                  zds_trajectory_data_t &track_data,
                                  const int &dir)
{
    if (path.empty())
        return false;

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return readRouteTRK(stream, track_data, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteTRK(std::ifstream &stream,
                                  zds_trajectory_data_t &track_data,
                                  const int &dir)
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
        // Если команда в поле arrows пустая, в неё распарситься
        // следующая переменная, проверяем это
        if (((track.arrows == "0") || (track.arrows == "3") || (track.arrows == "25")) &&
            (track.voltage != 0) && (track.voltage != 3) && (track.voltage != 25) &&
            (track.ordinate == 0))
        {
            track.ordinate = track.voltage;
            if (track.arrows == "0")
                track.voltage = 0;
            if (track.arrows == "3")
                track.voltage = 3;
            if (track.arrows == "25")
                track.voltage = 25;
            track.arrows = "";
        }
        // У команд n:x.x или m:x.x дробная чать может распарситься
        // как следующая переменная, проверяем это
        if ((track.arrows.front() == 'n' || (track.arrows.front() == 'm')) &&
            (track.voltage != 0) &&
            (track.ordinate == 0 || track.ordinate == 3 || track.ordinate == 25))
        {
            std::stringstream s;
            s << track.voltage;
            track.arrows = track.arrows + "." + s.str();
            track.voltage = track.ordinate;
            ss >> track.ordinate;
        }
        // Иногда встречается пикетаж со знаком минус
        if (track.ordinate < 0)
        {
            track.ordinate = -track.ordinate;
        }
        // В prev_uid хранится id предыдущего трека, но с нумерацией с 1
        // Можно использовать как индекс данного трека в массиве, где нумерация с 0
        // Только меняем у первого трека prev_uid с -1 на 0
        if (track.prev_uid == -1)
            track.prev_uid = 0;

        tmp_data.push_back(track);
    }

    auto it = tmp_data.begin();
    double trajectory_length = 0.0;

    int railway_coord_section = 0;
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
            size_t next_id = static_cast<size_t>(cur_track.next_uid - 1);
            zds_track_t next_track = tmp_data.at(next_id);
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

        cur_track.route_coord = trajectory_length;
        trajectory_length += cur_track.length;

        cur_track.railway_coord_section = railway_coord_section;

        if (not_end)
        {
            // Сохраняем координату в начале трека, если не дробные
            if (railway_coord_recalc_count == 0)
                railway_coord_recalc_begin = static_cast<double>(cur_track.ordinate);

            // Записываем координату в начале трека
            cur_track.railway_coord = railway_coord_recalc_begin;

            // Изменение пикетажа от начала данного трека
            // или от начала дробных подтреков
            double railway_coord_change = cur_track.railway_coord_end - railway_coord_recalc_begin;

            // В дробных треках ZDS записана одинаковая координата для всех подтреков
            if (abs(railway_coord_change) < 0.01)
            {
                // Считаем количество и суммарную длину подтреков
                railway_coord_recalc_count++;
                trajectory_recalc_length += cur_track.length;
            }
            else
            {
                // Сохраняем предыдущее направление увеличения координаты
                double prev_coord_increase_direction = coord_increase_direction;

                // Определяем направление увеличения координаты
                if (railway_coord_change > 0.0)
                {
                    coord_increase_direction = 1.0;
                }
                else
                {
                    coord_increase_direction = -1.0;
                    railway_coord_change = -railway_coord_change;
                }

                // Проверяем случай перехода на участок с новым пикетажем:
                // сравниваем изменение пикетажа с длиной данного трека
                // или суммой длин подтреков
                trajectory_recalc_length += cur_track.length;

                if (abs(railway_coord_change - trajectory_recalc_length) > 10.0)
                {
                    // Если разница велика, считаем,
                    // что со следующего трека начался новый пикетаж
                    ++railway_coord_section;

                    // А определённое по этой разнице
                    // направление увеличения координаты не имеет смысла,
                    // отменяем его
                    coord_increase_direction = prev_coord_increase_direction;

                    // Конец данного трека будет с координатой,
                    // увеличенной на длину данного трека или сумму длин подтреков
                    cur_track.railway_coord_end = railway_coord_recalc_begin +
                        coord_increase_direction * trajectory_recalc_length;

                    // Округляем координату
                    cur_track.railway_coord_end = 5.0 * std::round(cur_track.railway_coord_end / 5.0);
                }

                // Пересчитываем координаты дробных подтреков
                if (railway_coord_recalc_count)
                {
                    railway_coord_change = coord_increase_direction *
                        (cur_track.railway_coord_end - railway_coord_recalc_begin) /
                        static_cast<double>(railway_coord_recalc_count + 1);

                    // Также сбрасываем запись в поле arrows,
                    // поскольку она просто дублирует первый подтрек,
                    // оставляем только если это число (радиус кривой)
                    bool is_digit = false;
                    if (cur_track.arrows.size() > 2)
                    {
                        is_digit = true;
                        bool first_symbol = true;
                        for (auto c : cur_track.arrows)
                        {
                            bool c_is_digit = std::isdigit(c);
                            if (first_symbol)
                            {
                                first_symbol = false;
                                c_is_digit |= (c == '-');
                            }
                            is_digit &= c_is_digit;
                        }
                    }

                    // Стрелки, которые завершаются в конце трека,
                    // сохраняем у последнего подтрека (текущего)
                    bool is_last = ((cur_track.arrows  == "2+2") ||
                                    (cur_track.arrows == "2+") ||
                                    (cur_track.arrows == "2-"));

                    int last_track_id = track_data.size();
                    for (int i = 1; i <= railway_coord_recalc_count; ++i)
                    {
                        int id = last_track_id - i;
                        double new_coord = cur_track.railway_coord_end - i * railway_coord_change;
                        if (i == 1)
                        {
                            cur_track.railway_coord = new_coord;

                            if (!(is_last || is_digit))
                            {
                                cur_track.arrows = "";
                            }
                        }
                        else
                        {
                            track_data[id + 1].railway_coord = new_coord;

                            if (!is_digit)
                            {
                                track_data[id + 1].arrows = "";
                            }
                            if (is_last)
                            {
                                track_data[id].arrows = "";
                            }
                        }

                        track_data[id].railway_coord_end = new_coord;
                    }
                }

                // Вне дробных подтреков сбрасываем счётчик
                railway_coord_recalc_count = 0;
                trajectory_recalc_length = 0.0;
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

    if (dir > 0)
        railway_coord_sections1 = railway_coord_section + 1;
    else
        railway_coord_sections2 = railway_coord_section + 1;

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

        stream << track.route_coord / 1000.0f << " "
               << track.orth.z * 1000.0f << " "
               << track.curvature << std::endl;
    }

    stream.close();
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

        p_line_emem.railway_coord = track.route_coord;
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
