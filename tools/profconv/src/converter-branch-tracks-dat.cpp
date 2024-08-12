#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(const std::string &path, zds_branch_track_data_t &branch_data, const int &dir)
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
    return readBranchTracksDAT(stream, branch_data, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(QTextStream &stream, zds_branch_track_data_t &branch_data, const int &dir)
{
    zds_branch_track_t branch_track;
    bool is_next_from_other_main = false;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.isEmpty())
            continue;

        if (line.at(0) == ';')
            continue;

        QStringList tokens = line.split('\t');

        bool is_valid_value = false;
        int id_value = tokens[0].toInt(&is_valid_value);
        if ((!is_valid_value) || (id_value < 1) || (static_cast<size_t>(id_value) > tracks_data1.size()) || (tokens.size() < 2))
            continue;

        is_valid_value = false;
        double bias_value = tokens[1].toDouble(&is_valid_value);
        if ((!is_valid_value) || (abs(bias_value) > 100.0))
            continue;

        zds_branch_point_t branch_point;
        branch_point.main_track_id = id_value - 1;
        branch_point.bias = (abs(bias_value) < 0.01) ? 0.0 : bias_value;
        if (tokens.size() > 2)
            branch_point.signal_liter = tokens[2].toStdString();
        if (tokens.size() > 3)
            branch_point.signal_special = tokens[3].toStdString();
        branch_point.dir = dir;

        // Если какая-то из предыдущих точек была съездом на соседний главный,
        // ищем только обратный съезд (с нулевым смещением),
        // остальные случаи игнорируем
        if (is_next_from_other_main)
        {
            if ((abs(bias_value) >= 0.01))
                continue;

            is_next_from_other_main = false;

            // Ищем точку начала обратного съезда, считаем это съездом "2+2"
            findFromOtherMain(&branch_point);
            continue;
        }

        // Если смещение отрицательно - оно в сторону соседнего главного,
        // проверяем, что это может быть съезд "2-2" между главными
        if ((bias_value <= -0.01) && (branch_track.branch_points.empty()))
        {
            if (checkIsToOtherMain(&branch_point))
            {
                is_next_from_other_main = true;

                // Очистка
                branch_track.branch_points.clear();
                continue;
            }
            is_next_from_other_main = false;
        }

        branch_track.branch_points.push_back(branch_point);

        if (abs(bias_value) < 0.01)
        {
            if (branch_track.branch_points.size() > 1)
            {
                zds_branch_track_t *tmp_branch_track = new zds_branch_track_t(branch_track);
                branch_data.push_back(tmp_branch_track);
            }

            // Очистка
            branch_track.branch_points.clear();
            is_next_from_other_main = false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::checkIsToOtherMain(zds_branch_point_t *branch_point)
{
    double bias = branch_point->bias;
    size_t id_begin = branch_point->main_track_id;
    int dir = branch_point->dir;
    dvec3 point_after_bias;
    zds_track_t track;
    float coord;
    bool near_end;

    // Безусловный съезд "2-2" на соседний путь константой -7.47
    if (abs(bias - ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK) < 0.01)
    {
        if (dir > 0)
        {
            if (tracks_data2.empty())
                return false;

            point_after_bias = tracks_data1[id_begin].begin_point +
                               tracks_data1[id_begin].orth * 100.0 +
                               tracks_data1[id_begin].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;

            track = getNearestTrack(point_after_bias, tracks_data2, coord);
        }
        else
        {
            point_after_bias = tracks_data2[id_begin].end_point -
                               tracks_data2[id_begin].orth * 100.0 -
                               tracks_data2[id_begin].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;
            track = getNearestTrack(point_after_bias, tracks_data1, coord);
        }

        near_end = (coord > (track.trajectory_coord + 0.5 * track.length));
    }
    else
    {
        // Проверяем, если примерная точка после отклонения близка
        // к соседнему пути, считаем это съездом "2-2"
        if (dir > 0)
        {
            if (tracks_data2.empty())
                return false;

            point_after_bias = tracks_data1[id_begin].begin_point +
                               tracks_data1[id_begin].orth * 100.0 +
                               tracks_data1[id_begin].right * bias;

            track = getNearestTrack(point_after_bias, tracks_data2, coord);
        }
        else
        {
            point_after_bias = tracks_data2[id_begin].end_point -
                               tracks_data2[id_begin].orth * 100.0 -
                               tracks_data2[id_begin].right * bias;

            track = getNearestTrack(point_after_bias, tracks_data1, coord);
        }

        near_end = (coord > (track.trajectory_coord + 0.5 * track.length));

        dvec3 point_at_other_track = near_end ? track.end_point : track.begin_point;
        double distance = length(point_after_bias - point_at_other_track);
        if (distance > 3.0)
            return false;
    }

    zds_branch_2_2_t branch_2minus2 = zds_branch_2_2_t();
    if (dir > 0)
    {
        int id1 = id_begin;
        int id2 = near_end ? track.prev_uid + 1 : track.prev_uid;
        branch_2minus2.branch_point_fwd = *branch_point;
        branch_2minus2.is_fwd = true;
        branch_2minus2.id1 = id1;
        branch_2minus2.id2 = id2;
        branch_2minus2_data.push_back(new zds_branch_2_2_t(branch_2minus2));
    }
    else
    {
        int id1 = near_end ? track.prev_uid + 1 : track.prev_uid;
        int id2 = id_begin + 1;
        for (auto exist_branch : branch_2minus2_data)
        {
            if ((exist_branch->id1 == id1) && (exist_branch->id2 == id2))
            {
                exist_branch->branch_point_bwd = *branch_point;
                exist_branch->is_bwd = true;
                return true;
            }
        }

        branch_2minus2.branch_point_bwd = *branch_point;
        branch_2minus2.is_bwd = true;
        branch_2minus2.id1 = id1;
        branch_2minus2.id2 = id2;
        branch_2minus2_data.push_back(new zds_branch_2_2_t(branch_2minus2));
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::findFromOtherMain(zds_branch_point_t *branch_point)
{
    size_t id_end = branch_point->main_track_id;
    int dir = branch_point->dir;
    dvec3 point_after_bias;
    zds_track_t track;
    float coord;
    bool near_end;
    if (dir > 0)
    {
        point_after_bias = tracks_data1[id_end].end_point -
                           tracks_data1[id_end].orth * 100.0 +
                           tracks_data1[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;

        track = getNearestTrack(point_after_bias, tracks_data2, coord);
    }
    else
    {
        point_after_bias = tracks_data1[id_end].begin_point +
                           tracks_data1[id_end].orth * 100.0 -
                           tracks_data1[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;

        track = getNearestTrack(point_after_bias, tracks_data1, coord);
    }

    near_end = (coord > (track.trajectory_coord + 0.5 * track.length));

    zds_branch_2_2_t branch_2plus2 = zds_branch_2_2_t();
    if (dir > 0)
    {
        int id1 = id_end + 1;
        int id2 = near_end ? track.prev_uid + 1 : track.prev_uid;
        branch_2plus2.branch_point_fwd = *branch_point;
        branch_2plus2.is_fwd = true;
        branch_2plus2.id1 = id1;
        branch_2plus2.id2 = id2;
        branch_2plus2_data.push_back(new zds_branch_2_2_t(branch_2plus2));
    }
    else
    {
        int id1 = near_end ? track.prev_uid + 1 : track.prev_uid;
        int id2 = id_end;
        for (auto exist_branch : branch_2plus2_data)
        {
            if ((exist_branch->id1 == id1) && (exist_branch->id2 == id2))
            {
                exist_branch->branch_point_bwd = *branch_point;
                exist_branch->is_bwd = true;
                return;
            }
        }

        branch_2plus2.branch_point_bwd = *branch_point;
        branch_2plus2.is_bwd = true;
        branch_2plus2.id1 = id1;
        branch_2plus2.id2 = id2;
        branch_2plus2_data.push_back(new zds_branch_2_2_t(branch_2plus2));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::calcBranchTrack1(zds_branch_track_t *branch_track)
{
    // Параметры следующего смещения
    size_t id_begin = branch_track->branch_points.begin()->main_track_id;
    double bias_prev = 0.0;
    double bias_curr = branch_track->branch_points.begin()->bias;
    double coord_begin = tracks_data1[id_begin].trajectory_coord;
    // Первая точка
    calculated_branch_point_t point_begin;
    point_begin.point = tracks_data1[id_begin].begin_point;
    point_begin.trajectory_coord = 0.0;
    point_begin.railway_coord = tracks_data1[id_begin].railway_coord;
    // Добавляем первую точку
    branch_track->branch_trajectory.push_back(point_begin);

    for (auto it = branch_track->branch_points.begin(); it != branch_track->branch_points.end(); ++it)
    {
        // Проверяем, если следующей написана дублирующая точка, игнорируем эту
        if ((it+1) != branch_track->branch_points.end())
        {
            if ((it+1)->main_track_id == id_begin)
            {
                ++it;
                bias_curr = (it)->bias;
                continue;
            }
        }

        // Траектория отклонения
        // Отклонение в ZDS длится 100 метров
        // На всякий случай проверяем разбиение стометрового трека на подтреки
        // Ищем трек, который через 100 м - как трек, который хотя бы через 95 м
        size_t id = id_begin;
        double main_traj_l = 0.0;
        do
        {
            ++id;
            main_traj_l = tracks_data1[id].trajectory_coord - coord_begin;
        }
        while (main_traj_l < 95.0);
        size_t id_end = id;
        double railway_coord_length = tracks_data1[id_end].railway_coord - tracks_data1[id_begin].railway_coord;

        // Расчёт промежуточных точек по траектории сплайна отклонения
        id = id_begin;
        for (size_t i = 0; i < NUM_BIAS_POINTS; ++i)
        {
            double coord_add = COORD_COEFF[i] * main_traj_l;
            double coord_ref = coord_begin + coord_add;
            // Находим нужный подтрек - у следующего координата больше
            while (tracks_data1[id + 1].trajectory_coord < coord_ref)
            {
                ++id;
            }

            dvec3 main_track_point = tracks_data1[id].begin_point +
                                     tracks_data1[id].orth * (coord_ref - tracks_data1[id].trajectory_coord);

            // Промежуточная точка отклонения
            calculated_branch_point_t point;
            double bias_coeff = bias_prev * (1.0 - BIAS_COEFF[i]) +
                                bias_curr * BIAS_COEFF[i];
            point.point = main_track_point +
                          tracks_data1[id].right * bias_coeff;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data1[id_begin].railway_coord +
                                  COORD_COEFF[i] * railway_coord_length;
            // Добавляем промежуточную точку отклонения
            branch_track->branch_trajectory.push_back(point);
        }

        // Если смещение нулевое - траектория завершена
        if ((bias_curr == 0.0) || ((it+1) == branch_track->branch_points.end()))
        {
            // Дописываем последнюю точку
            calculated_branch_point_t point;
            point.point = tracks_data1[id_end].begin_point;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data1[id_end].railway_coord;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);
            return true;
        }

        // === TODO ===
        // Разделение траектории по светофору
        // === TODO ===

        // Расчёт точек траектории, параллельной главному пути, со смещением
        for (id = id_end; id <= (it+1)->main_track_id; ++id)
        {
            dvec3 mean_right = normalize(tracks_data1[id].right +
                                         tracks_data1[id - 1].right);
            calculated_branch_point_t point;
            point.point = tracks_data1[id].begin_point +
                          mean_right * bias_curr;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data1[id].railway_coord;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);
        }

        // Подготавливаемся к следующей итерации
        id_begin = (it+1)->main_track_id;
        bias_prev = bias_curr;
        bias_curr = (it+1)->bias;
        coord_begin = tracks_data1[id_begin].trajectory_coord;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::calcBranchTrack2(zds_branch_track_t *branch_track)
{
    // Параметры следующего смещения
    size_t id_begin = branch_track->branch_points.begin()->main_track_id + 1;
    double bias_prev = 0.0;
    double bias_curr = branch_track->branch_points.begin()->bias;
    double coord_begin = tracks_data2[id_begin].trajectory_coord;
    // Первая точка
    calculated_branch_point_t point_begin;
    point_begin.point = tracks_data2[id_begin].begin_point;
    point_begin.trajectory_coord = 0.0;
    point_begin.railway_coord = tracks_data2[id_begin].railway_coord;
    // Добавляем первую точку
    branch_track->branch_trajectory.push_back(point_begin);

    for (auto it = branch_track->branch_points.begin(); it != branch_track->branch_points.end(); ++it)
    {
        // Проверяем, если следующей написана дублирующая точка, игнорируем эту
        if ((it+1) != branch_track->branch_points.end())
        {
            if ((it+1)->main_track_id + 1 == id_begin)
            {
                ++it;
                bias_curr = (it)->bias;
                continue;
            }
        }

        // Траектория отклонения
        // Отклонение в ZDS длится 100 метров
        // На всякий случай проверяем разбиение стометрового трека на подтреки
        // Ищем трек, который через 100 м - как трек, который хотя бы через 95 м
        size_t id = id_begin;
        double main_traj_l = 0.0;
        do
        {
            --id;
            main_traj_l = coord_begin - tracks_data2[id].trajectory_coord;
        }
        while (main_traj_l < 95.0);
        size_t id_end = id;
        double railway_coord_length = tracks_data2[id_end].railway_coord - tracks_data2[id_begin].railway_coord;

        // Расчёт промежуточных точек по траектории сплайна отклонения
        id = id_begin;
        for (size_t i = 0; i < NUM_BIAS_POINTS; ++i)
        {
            double coord_add = COORD_COEFF[i] * main_traj_l;
            double coord_ref = coord_begin - coord_add;
            // Находим нужный подтрек - у него координата меньше
            while (coord_ref < tracks_data2[id].trajectory_coord)
            {
                --id;
            }

            dvec3 main_track_point = tracks_data2[id].begin_point +
                                     tracks_data2[id].orth * (coord_ref - tracks_data2[id].trajectory_coord);

            // Промежуточная точка отклонения
            calculated_branch_point_t point;
            double bias_coeff = bias_prev * (1.0 - BIAS_COEFF[i]) +
                                bias_curr * BIAS_COEFF[i];
            point.point = main_track_point -
                          tracks_data2[id].right * bias_coeff;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data2[id_begin].railway_coord +
                                  COORD_COEFF[i] * railway_coord_length;
            // Добавляем промежуточную точку отклонения
            branch_track->branch_trajectory.push_back(point);
        }

        // Если смещение нулевое - траектория завершена
        if ((bias_curr == 0.0) || ((it+1) == branch_track->branch_points.end()))
        {
            // Дописываем последнюю точку
            calculated_branch_point_t point;
            point.point = tracks_data2[id_end].begin_point -
                          tracks_data2[id_end].right * bias_curr;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data2[id_end].railway_coord;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);
            return true;
        }

        // === TODO ===
        // Разделение траектории по светофору
        // === TODO ===

        // Расчёт точек траектории, параллельной главному пути, со смещением
        for (id = id_end; id > (it+1)->main_track_id; --id)
        {
            dvec3 mean_right = normalize(tracks_data2[id].right +
                                         tracks_data2[id - 1].right);
            calculated_branch_point_t point;
            point.point = tracks_data2[id].begin_point -
                          mean_right * bias_curr;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data2[id].railway_coord;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);
        }

        // Подготавливаемся к следующей итерации
        id_begin = (it+1)->main_track_id + 1;
        bias_prev = bias_curr;
        bias_curr = (it+1)->bias;
        coord_begin = tracks_data2[id_begin].trajectory_coord;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::calcBranch22(zds_branch_2_2_t *branch22)
{
    // Первая точка
    calculated_branch_point_t point_begin;
    point_begin.point = tracks_data1[branch22->id1].begin_point;
    point_begin.railway_coord = tracks_data1[branch22->id1].railway_coord;
    // Добавляем первую точку
    point_begin.trajectory_coord = 0.0;
    branch22->branch_trajectory.push_back(point_begin);

    // Последняя точка
    calculated_branch_point_t point_end;
    point_end.point = tracks_data2[branch22->id2].begin_point;
    point_end.railway_coord = tracks_data2[branch22->id2].railway_coord;

    // Параметры отклонения
    dvec3 begin_to_end = point_end.point - point_begin.point;
    dvec3 mean_orth = normalize(tracks_data1[branch22->id1].orth +
                                tracks_data2[branch22->id2].orth);
    dvec3 mean_right = normalize(tracks_data1[branch22->id1].right +
                                 tracks_data2[branch22->id2].right);
    double coord_length = dot(begin_to_end, mean_orth);
    double bias = dot(begin_to_end, mean_right);
    double railway_coord_length = point_end.railway_coord - point_begin.railway_coord;

    for (size_t i = 0; i < NUM_BIAS_POINTS; ++i)
    {
        // Промежуточная точка отклонения
        calculated_branch_point_t point;
        point.point = point_begin.point +
                      mean_orth * COORD_COEFF[i] * coord_length +
                      mean_right * BIAS_COEFF[i] * bias;
        double l = length(point.point - branch22->branch_trajectory.back().point);
        point.trajectory_coord = branch22->branch_trajectory.back().trajectory_coord + l;
        point.railway_coord = point_begin.railway_coord +
                              COORD_COEFF[i] * railway_coord_length;
        // Добавляем промежуточную точку отклонения
        branch22->branch_trajectory.push_back(point);
    }

    // Дописываем последнюю точку
    double l = length(point_end.point - branch22->branch_trajectory.back().point);
    point_end.trajectory_coord = branch22->branch_trajectory.back().trajectory_coord + l;
    // Добавляем точку траектории
    branch22->branch_trajectory.push_back(point_end);
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeBranchTrajectory(const std::string &filename, const std::vector<calculated_branch_point_t> *branch_trajectory)
{
    std::string path = compinePath(toNativeSeparators(topologyDir), filename);

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + ".bak").c_str()) );

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto it = branch_trajectory->begin(); it != branch_trajectory->end(); ++it)
    {
        stream << (*it).point.x << ";"
               << (*it).point.y << ";"
               << (*it).point.z << ";"
               << static_cast<int>(round((*it).railway_coord)) << ";"
               << (*it).trajectory_coord << "\n";
    }

    file.close();
}
