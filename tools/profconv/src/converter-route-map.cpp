#include    "converter.h"

#include    <QFile>
#include    <iostream>
#include    <sstream>

#include    "path-utils.h"


void add_element(float x, std::vector<float> &array)
{
    float eps = 1.0f;

    if (array.size() != 0)
    {
        float last = *(array.end() - 1);

        if ( abs(last - x) >= eps )
            array.push_back(x);
    }
    else
    {
        array.push_back(x);
    }
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteMAP(const std::string &path, zds_route_map_data_t &map_data)
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
    return readRouteMAP(stream, map_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readRouteMAP(QTextStream &stream, zds_route_map_data_t &map_data)
{
    bool may_add_info_to_last = false;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        // Пустое название объекта
        if (*(line.begin()) == ',')
            continue;

        // Строка с объектом заканчивается точкой с запятой, а если нет,
        // это может быть строка с информацией об объекте из предыдущей строки
        if (*(line.end() - 1) != ';')
        {
            if (may_add_info_to_last)
            {
                map_data.back()->obj_info = line.toStdString();
                may_add_info_to_last = false;
            }
            continue;
        }

        // Удаляем завершающую точку с запятой и разделяем строку на данные
        QStringList tokens = line.removeLast().split(',');
        if (tokens.size() < 7)
            continue;

        // Читаем информацию об объекте
        zds_object_position_t zds_object = zds_object_position_t();
        zds_object.obj_name = tokens[0].toStdString();
        zds_object.position.x = tokens[1].toDouble();
        zds_object.position.y = tokens[2].toDouble();
        zds_object.position.z = tokens[3].toDouble();
        zds_object.attitude.x = tokens[4].toDouble();
        zds_object.attitude.y = tokens[5].toDouble();
        zds_object.attitude.z = tokens[6].toDouble();

        map_data.push_back(new zds_object_position_t(zds_object));
        may_add_info_to_last = true;
    }

    return (!map_data.empty());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::findSignalsAtMap()
{
    for (auto zds_obj : route_map_data)
    {
        // Игнорируем модели с пустой дополнительной информацией
        // поскольку нас интересуют светофоры с литерой
        if (zds_obj->obj_info.empty())
            continue;

        // Проходной, предвходной
        if ((zds_obj->obj_name == "signal_line") ||
            (zds_obj->obj_name == "signal_pred"))
        {
            zds_signal_position_t *signal = new zds_signal_position_t();
            signal->obj_name = zds_obj->obj_name;
            signal->position = zds_obj->position;
            signal->attitude = zds_obj->attitude;

            signal->type = "ab3line";
            signal->liter = zds_obj->obj_info;
            signals_line_data.push_back(signal);
        }

        // Входной
        if (zds_obj->obj_name == "signal_enter")
        {
            zds_signal_position_t *signal = new zds_signal_position_t();
            signal->obj_name = zds_obj->obj_name;
            signal->position = zds_obj->position;
            signal->attitude = zds_obj->attitude;

            signal->type = "ab3entr";
            signal->liter = zds_obj->obj_info;
            signals_enter_data.push_back(signal);
        }

        // Маршрутный/выходной, карликовый 5 линз, карликовый 3 линзы
        if ((zds_obj->obj_name == "signal_exit") ||
            (zds_obj->obj_name == "sig_k5p") ||
            (zds_obj->obj_name == "sig_k3p"))
        {
            zds_signal_position_t *signal = new zds_signal_position_t();
            signal->obj_name = zds_obj->obj_name;
            signal->position = zds_obj->position;
            signal->attitude = zds_obj->attitude;

            signal->type = "ab3exit";
            signal->liter = zds_obj->obj_info;
            signals_exit_data.push_back(signal);
        }

        // Повторительный, карликовый повторительный
        if ((zds_obj->obj_name == "sig_povt") ||
            (zds_obj->obj_name == "sig_povt_k"))
        {
            zds_signal_position_t *signal = new zds_signal_position_t();
            signal->obj_name = zds_obj->obj_name;
            signal->position = zds_obj->position;
            signal->attitude = zds_obj->attitude;

            signal->liter = zds_obj->obj_info;
            signals_povt_data.push_back(signal);
        }

        // Маневровый мачтовый, маневровый карликовый
        if ((zds_obj->obj_name == "sig_m2m") ||
            (zds_obj->obj_name == "sig_k2m"))
        {
            zds_signal_position_t *signal = new zds_signal_position_t();
            signal->obj_name = zds_obj->obj_name;
            signal->position = zds_obj->position;
            signal->attitude = zds_obj->attitude;

            signal->liter = zds_obj->obj_info;
            signals_maneurous_data.push_back(signal);
        }
    }

    // Привязка к главным путям
    for (auto map : {signals_line_data,
                     signals_enter_data,
                     signals_exit_data,
                     signals_povt_data,
                     signals_maneurous_data})
    {
        for (auto sig : map)
        {
            if (findTrackNearToSignal(sig, 1))
                continue;

            if (findTrackNearToSignal(sig, -1))
                continue;
        }
    }

    // Вывод для отладки
    std::string path_train = compinePath(topologyDir, "signals_at_map_main_track.conf");
    std::string path_povt = compinePath(topologyDir, "signals_at_map_main_track_povt.conf");
    std::string path_maneurous = compinePath(topologyDir, "signals_at_map_main_track_maneurous.conf");
    std::string path_not_at_main = compinePath(topologyDir, "signals_at_map_not_main_track.conf");

    QFile file_t(QString(path_train.c_str()));
    if (!file_t.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QFile file_p(QString(path_povt.c_str()));
    if (!file_p.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QFile file_m(QString(path_maneurous.c_str()));
    if (!file_m.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QFile file_n(QString(path_not_at_main.c_str()));
    if (!file_n.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream_t(&file_t);
    stream_t.setEncoding(QStringConverter::Utf8);
    stream_t.setRealNumberNotation(QTextStream::FixedNotation);
    QTextStream stream_p(&file_p);
    stream_p.setEncoding(QStringConverter::Utf8);
    stream_p.setRealNumberNotation(QTextStream::FixedNotation);
    QTextStream stream_m(&file_m);
    stream_m.setEncoding(QStringConverter::Utf8);
    stream_m.setRealNumberNotation(QTextStream::FixedNotation);
    QTextStream stream_n(&file_n);
    stream_n.setEncoding(QStringConverter::Utf8);
    stream_n.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto map : {signals_line_data,
                     signals_enter_data,
                     signals_exit_data/*,
                     signals_povt_data,
                     signals_maneurous_data*/})
    {
        for (auto sig : map)
        {
            if (sig->route_num == -1)
            {
                stream_n << sig->obj_name.c_str()
                         << DELIMITER_SYMBOL << sig->liter.c_str()
                         << DELIMITER_SYMBOL << sig->position.x
                         << DELIMITER_SYMBOL << sig->position.y
                         << DELIMITER_SYMBOL << sig->position.z
                         << DELIMITER_SYMBOL << sig->attitude.x
                         << DELIMITER_SYMBOL << sig->attitude.y
                         << DELIMITER_SYMBOL << sig->attitude.z;
                stream_n << "\n"
                        << "NO_AT_MAIN"
                        << DELIMITER_SYMBOL
                        << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                        << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                        << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                        << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                        << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                        << "\n";
            }
            else
            {
                stream_t << sig->obj_name.c_str()
                         << DELIMITER_SYMBOL << sig->liter.c_str()
                         << DELIMITER_SYMBOL << sig->position.x
                         << DELIMITER_SYMBOL << sig->position.y
                         << DELIMITER_SYMBOL << sig->position.z
                         << DELIMITER_SYMBOL << sig->attitude.x
                         << DELIMITER_SYMBOL << sig->attitude.y
                         << DELIMITER_SYMBOL << sig->attitude.z;
                stream_t << "\n"
                        << "MAIN:";
                if (sig->route_num == 1)
                    stream_t << "route1";
                if (sig->route_num == 2)
                    stream_t << "route2";

                stream_t << DELIMITER_SYMBOL
                        << DELIMITER_SYMBOL
                        << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                        << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                        << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                        << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                        << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                        << DELIMITER_SYMBOL << "coord " << sig->track_coord
                        << DELIMITER_SYMBOL << "dir" << sig->direction
                        << DELIMITER_SYMBOL << "track" << sig->track_id
                        << "\n";
            }
        }
    }

    for (auto sig : signals_povt_data)
    {
        stream_p << sig->obj_name.c_str()
                 << DELIMITER_SYMBOL << sig->liter.c_str()
                 << DELIMITER_SYMBOL << sig->position.x
                 << DELIMITER_SYMBOL << sig->position.y
                 << DELIMITER_SYMBOL << sig->position.z
                 << DELIMITER_SYMBOL << sig->attitude.x
                 << DELIMITER_SYMBOL << sig->attitude.y
                 << DELIMITER_SYMBOL << sig->attitude.z;
        if (sig->route_num == -1)
        {
            stream_p << "\n"
                     << "NO_AT_MAIN"
                     << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                     << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                     << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                     << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                     << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                     << "\n";
        }
        else
        {
            stream_p << "\n"
                     << "MAIN:";
            if (sig->route_num == 1)
                stream_p << "route1";
            if (sig->route_num == 2)
                stream_p << "route2";

            stream_p << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                     << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                     << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                     << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                     << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                     << DELIMITER_SYMBOL << "coord " << sig->track_coord
                     << DELIMITER_SYMBOL << "dir" << sig->direction
                     << DELIMITER_SYMBOL << "track" << sig->track_id
                     << "\n";
        }
    }

    for (auto sig : signals_maneurous_data)
    {
        stream_m << sig->obj_name.c_str()
                 << DELIMITER_SYMBOL << sig->liter.c_str()
                 << DELIMITER_SYMBOL << sig->position.x
                 << DELIMITER_SYMBOL << sig->position.y
                 << DELIMITER_SYMBOL << sig->position.z
                 << DELIMITER_SYMBOL << sig->attitude.x
                 << DELIMITER_SYMBOL << sig->attitude.y
                 << DELIMITER_SYMBOL << sig->attitude.z;
        if (sig->route_num == -1)
        {
            stream_m << "\n"
                     << "NO_AT_MAIN"
                     << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                     << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                     << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                     << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                     << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                     << "\n";
        }
        else
        {
            stream_m << "\n"
                     << "MAIN:";
            if (sig->route_num == 1)
                stream_m << "route1";
            if (sig->route_num == 2)
                stream_m << "route2";

            stream_m << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL
                     << DELIMITER_SYMBOL << "sig{" << sig->position.x << "," << sig->position.y << "}"
                     << DELIMITER_SYMBOL << "track{" << sig->nearest_point.x << "," << sig->nearest_point.y << "}"
                     << DELIMITER_SYMBOL << "right{" << sig->track_right.x << "," << sig->track_right.y << "}"
                     << DELIMITER_SYMBOL << "rho{" << sig->rho_right.x << "," << sig->rho_right.y << "}"
                     << DELIMITER_SYMBOL << "dist  " << sig->distance_from_main
                     << DELIMITER_SYMBOL << "coord " << sig->track_coord
                     << DELIMITER_SYMBOL << "dir" << sig->direction
                     << DELIMITER_SYMBOL << "track" << sig->track_id
                     << "\n";
        }
    }

    file_t.close();
    file_p.close();
    file_m.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::findTrackNearToSignal(zds_signal_position_t *signal, int dir)
{
    // Треки главного пути
    zds_trajectory_data_t* tracks_data = (dir > 0) ? &tracks_data1 : &tracks_data2;

    // Трек главного пути напротив светофора
    float coord;
    zds_track_t nearest_track = getNearestTrack(signal->position, *tracks_data, coord);
    bool near_end = (coord > (nearest_track.route_coord + 0.5 * nearest_track.length));

    // Направление светофора
    signal->direction = 0;
    double signal_attitude_z = signal->attitude.z;
    while (abs(signal_attitude_z) > 180.0)
    {
        signal_attitude_z += 360.0 * ((signal_attitude_z < 0) ? 1 : -1);
    }
    double track_attitude_z = (nearest_track.orth.x > 0.0) ? acos(nearest_track.orth.y) : - acos(nearest_track.orth.y);
    track_attitude_z = track_attitude_z * 180.0 / 3.1415926535898;
    signal->attitude_z_angle = track_attitude_z - signal_attitude_z;
    // Допускаем поворот светофора к траектории до 45 градусов, определяем направление
    if (abs(signal->attitude_z_angle) < 45.0)
    {
        signal->direction = 1;
    }
    if (abs(signal->attitude_z_angle) > 135.0)
    {
        signal->direction = -1;
    }
    // Светофор в непонятном направлении - дальше делать нечего
    if (signal->direction == 0)
        return false;

    signal->attitude.z = signal_attitude_z;

    // Точка главного пути напротив светофора
    double track_coord = coord - nearest_track.route_coord;
    dvec3 nearest_point = nearest_track.begin_point + nearest_track.orth * track_coord;
    dvec3 rho_right = signal->position - nearest_point;
    double distance_right = dot(rho_right, nearest_track.right);

    signal->nearest_point = nearest_point;
    signal->rho_right = rho_right;
    signal->track_right = nearest_track.right;
    signal->distance_from_main = distance_right;

    // Проверяем, светофор слева ( < 0 ) или справа ( > 0 )
    if (distance_right < 0)
    {
        // Проверяем, светофор в направлении туда или нет
        if (signal->direction > 0)
        {
            // Светофор слева
            // Считаем, что относится к этому пути, если не далее 3 метров
            if (distance_right > -3.1)
            {
                signal->is_left = true;

                signal->route_num = (dir > 0) ? 1 : 2;
                signal->track_id = near_end ? nearest_track.prev_uid + 1 : nearest_track.prev_uid;
                signal->track_coord = near_end ? track_coord - nearest_track.length : track_coord;

                return true;
            }
        }
        else
        {
            // Светофор слева в направлении обратно - обычный светофор обратно
            // Считаем, что относится к этому пути, если не далее 4 метров
            if (distance_right > -4.1)
            {
                signal->route_num = (dir > 0) ? 1 : 2;
                signal->track_id = near_end ? nearest_track.prev_uid + 1 : nearest_track.prev_uid;
                signal->track_coord = near_end ? track_coord - nearest_track.length : track_coord;

                return true;
            }
        }
    }
    else
    {
        // Проверяем, светофор в направлении туда или нет
        if (signal->direction > 0)
        {
            // Обычный светофор справа
            // Считаем, что относится к этому пути, если не далее 4 метров
            if (distance_right < 4.1)
            {
                signal->route_num = (dir > 0) ? 1 : 2;
                signal->track_id = near_end ? nearest_track.prev_uid + 1 : nearest_track.prev_uid;
                signal->track_coord = near_end ? track_coord - nearest_track.length : track_coord;

                return true;
            }
        }
        else
        {
            // Светофор справа, но обратно - это светофор слева
            // Считаем, что относится к этому пути, если не далее 3 метров
            if (distance_right < 3.1)
            {
                signal->is_left = true;

                signal->route_num = (dir > 0) ? 1 : 2;
                signal->track_id = near_end ? nearest_track.prev_uid + 1 : nearest_track.prev_uid;
                signal->track_coord = near_end ? track_coord - nearest_track.length : track_coord;

                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::findNeutralInsertions(std::vector<neutral_insertion_t> ni)
{
    std::vector<float> begins;
    std::vector<float> ends;

    for (auto zds_object : route_map_data)
    {
        if (zds_object->obj_name == "nvne")
        {
            neutral_insertion_t n_insertion;

            float begin_coord = 0;
            zds_track_t track = getNearestTrack(zds_object->position, tracks_data1, begin_coord);

            add_element(begin_coord, begins);
        }

        if (zds_object->obj_name == "nvke")
        {
            neutral_insertion_t n_insertion;

            float end_coord = 0;
            zds_track_t track = getNearestTrack(zds_object->position, tracks_data1, end_coord);

            add_element(end_coord, ends);
        }
    }

    std::sort(begins.begin(), begins.end(), std::less<float>());
    std::sort(ends.begin(), ends.end(), std::less<float>());

    if (begins.size() == ends.size())
    {
        float max_len = 500.0f;

        for (size_t i = 0; i < begins.size(); ++i)
        {
            neutral_insertion_t n_ins;

            n_ins.begin_coord = begins[i];
            n_ins.end_coord = ends[i];
            n_ins.length = abs(n_ins.end_coord - n_ins.begin_coord);

            if (n_ins.length <= max_len)
                ni.push_back(n_ins);
        }
    }

    return ni.size() != 0;
}
