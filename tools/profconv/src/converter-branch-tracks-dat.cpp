#include    "converter.h"

#include    <iostream>
#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(const std::string &path, const int &dir)
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
    return readBranchTracksDAT(stream, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::readBranchTracksDAT(QTextStream &stream, const int &dir)
{
    std::vector<zds_branch_point_t*> tmp_data;

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

        zds_branch_point_t* branch_point = new zds_branch_point_t();
        branch_point->main_track_id = id_value - 1;
        branch_point->bias = (abs(bias_value) < 0.01) ? 0.0 : bias_value;
        if ((tokens.size() > 2) && (!tokens[2].isEmpty()))
        {
            // Светофор
            branch_point->signal_liter = tokens[2].toStdString();
            // Отмечаем необходимость разделить траекторию светофором
            // Пока не знаем id элемента трека, просто меняем на 1
            branch_point->id_split_point_by_signal = 1;
        }
        if ((tokens.size() > 3) && (!tokens[3].isEmpty()))
        {
            branch_point->signal_special = tokens[3].toStdString();
        }
        branch_point->dir = dir;

        // Проверяем, если id этой точки дублирует предыдущую,
        // а предыдущая не завершала траекторию нулевым смещением,
        // перезаписываем предыдущую
        if ( (!tmp_data.empty()) && (tmp_data.back()->bias != 0.0) && ((tmp_data.back()->main_track_id) == (id_value - 1)) )
        {
            tmp_data.back()->bias = branch_point->bias;
            if (branch_point->id_split_point_by_signal == 1)
            {
                tmp_data.back()->id_split_point_by_signal = branch_point->id_split_point_by_signal;
                tmp_data.back()->signal_liter = branch_point->signal_liter;
                tmp_data.back()->signal_special = branch_point->signal_special;
            }
        }
        else
        {
            tmp_data.push_back(branch_point);
        }
    }

    zds_branch_track_t branch_track = zds_branch_track_t();
    bool is_next_from_other_main = false;
    for (auto it = tmp_data.begin(); it != tmp_data.end(); ++it)
    {
        zds_branch_point_t* branch_point = *it;

        // Если предыдущая точка была на соседнем главном пути
        if (is_next_from_other_main)
        {
            // проверяем обратный съезд (с нулевым смещением)
            if (branch_point->bias == 0.0)
            {
                // Ищем точку начала обратного съезда, считаем это съездом "2+2"
                findFromOtherMain(branch_point);

                // Дальше мы не на соседнем главном
                is_next_from_other_main = false;
                continue;
            }

            // проверяем, что новое смещение всё ещё совпадает с соседним главным
            if (checkIsToOtherMain(branch_point, false))
            {
                is_next_from_other_main = true;
                continue;
            }

            // Создаём новую траекторию
            branch_track.begin_at_other = true;
            branch_track.branch_points.push_back(branch_point);

            // Дальше мы не на соседнем главном
            is_next_from_other_main = false;
            continue;
        }

        // Проверяем, что точка может быть съездом на соседний главный путь
        // Но сперва проверяем, что мы на новой траектории без предыдущих точек
        if (branch_track.branch_points.empty())
        {
            // Новую траекторию считаем съездом "2-2"
            if (checkIsToOtherMain(branch_point, true))
            {
                is_next_from_other_main = true;
                branch_track = zds_branch_track_t();
                continue;
            }

            // Если нет, то это не съезд на соседний главный
            is_next_from_other_main = false;
        }
        else
        {
            // Если мы на траектории с точками, проверяем попадание в соседний главный путь
            if (checkIsToOtherMain(branch_point, false) && (branch_point->bias != 0.0))
            {
                is_next_from_other_main = true;

                // Заканчиваем траекторию, попавшую в соседний главный путь
                branch_track.end_at_other = true;
                branch_track.branch_points.push_back(branch_point);
                zds_branch_track_t* tmp_branch = new zds_branch_track_t(branch_track);
                if (dir > 0)
                    calcBranchTrack1(tmp_branch);
                else
                    calcBranchTrack2(tmp_branch);

                // Очистка
                branch_track.branch_points.clear();
                branch_track = zds_branch_track_t();
                continue;
            }

            // Если нет, то это не съезд на соседний главный
            is_next_from_other_main = false;
        }

        // Добавляем точку в траекторию бокового пути
        branch_track.branch_points.push_back(branch_point);

        // Если смещение нулевое, заканчиваем траекторию
        if (branch_point->bias == 0.0)
        {
            // Считаем и добавляем траекторию
            if (branch_track.branch_points.size() > 1)
            {
                zds_branch_track_t* tmp_branch = new zds_branch_track_t(branch_track);
                if (dir > 0)
                    calcBranchTrack1(tmp_branch);
                else
                    calcBranchTrack2(tmp_branch);
            }

            // Очистка
            branch_track.branch_points.clear();
            branch_track = zds_branch_track_t();
            is_next_from_other_main = false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::checkIsToOtherMain(zds_branch_point_t* branch_point, bool is_add_2minus2)
{
    double bias = branch_point->bias;
    size_t id_begin = branch_point->main_track_id;
    int dir = branch_point->dir;
    dvec3 point_after_bias;
    zds_track_t track;
    float coord;
    bool near_end;

    // Безусловный перемещение на соседний путь константой -7.47
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

        near_end = (coord > (track.route_coord + 0.5 * track.length));
    }
    else
    {
        // Проверяем, если примерная точка после отклонения близка
        // к соседнему пути, считаем это перемещением на соседний путь
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

        near_end = (coord > (track.route_coord + 0.5 * track.length));

        dvec3 point_at_other_track = near_end ? track.end_point : track.begin_point;
        double distance = length(point_after_bias - point_at_other_track);
        if (distance > 3.0)
            return false;
    }

    if (!is_add_2minus2)
        return true;

    // Создаём съезд "2-2"
    zds_branch_2_2_t branch_2minus2 = zds_branch_2_2_t();
    if (dir > 0)
    {
        int id1 = id_begin;
        int id2 = near_end ? track.prev_uid + 1 : track.prev_uid;
        branch_2minus2.branch_point_fwd = *branch_point;
        branch_2minus2.is_fwd = true;
        branch_2minus2.id1 = id1;
        branch_2minus2.id2 = id2;
        calcBranch22(&branch_2minus2);
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
        calcBranch22(&branch_2minus2);
        branch_2minus2_data.push_back(new zds_branch_2_2_t(branch_2minus2));
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::findFromOtherMain(zds_branch_point_t* branch_point)
{
    size_t id_end = branch_point->main_track_id;
    int dir = branch_point->dir;
    dvec3 point_before_bias;
    zds_track_t track;
    float coord;

    if (dir > 0)
    {
        point_before_bias = tracks_data1[id_end].end_point -
                            tracks_data1[id_end].orth * 100.0 +
                            tracks_data1[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;

        track = getNearestTrack(point_before_bias, tracks_data2, coord);
    }
    else
    {
        point_before_bias = tracks_data2[id_end].begin_point +
                            tracks_data2[id_end].orth * 100.0 -
                            tracks_data2[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;

        track = getNearestTrack(point_before_bias, tracks_data1, coord);
    }

    bool near_end = (coord > (track.route_coord + 0.5 * track.length));

    zds_branch_2_2_t branch_2plus2 = zds_branch_2_2_t();
    if (dir > 0)
    {
        int id1 = id_end + 1;
        int id2 = near_end ? track.prev_uid + 1 : track.prev_uid;
        branch_2plus2.branch_point_fwd = *branch_point;
        branch_2plus2.is_fwd = true;
        branch_2plus2.id1 = id1;
        branch_2plus2.id2 = id2;
        calcBranch22(&branch_2plus2);
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
        calcBranch22(&branch_2plus2);
        branch_2plus2_data.push_back(new zds_branch_2_2_t(branch_2plus2));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::calcBranchTrack1(zds_branch_track_t* branch_track)
{
    auto branch_point_begin = branch_track->branch_points.begin();
    // Параметры следующего смещения
    size_t id_begin = (*branch_point_begin)->main_track_id;
    double bias_prev = 0.0;
    double bias_curr = (*branch_point_begin)->bias;
    double coord_begin = tracks_data1[id_begin].route_coord;

    // Первая точка
    calculated_branch_point_t point_begin;
    point_begin.point = tracks_data1[id_begin].begin_point;
    point_begin.trajectory_coord = 0.0;
    point_begin.railway_coord = tracks_data1[id_begin].railway_coord;
    branch_track->id_begin = id_begin;
    // Проверяем начало с соседнего пути
    if (branch_track->begin_at_other)
    {
        dvec3 point_after_bias = tracks_data1[id_begin].begin_point +
                                 tracks_data1[id_begin].right * bias_curr;
        float coord;
        zds_track_t track = getNearestTrack(point_after_bias, tracks_data2, coord);
        bool near_end = (coord > (track.route_coord + 0.5 * track.length));
        branch_track->id_begin_at_other = near_end ? track.prev_uid + 1 : track.prev_uid;

        point_begin.point = near_end ? track.end_point : track.begin_point;
        point_begin.railway_coord = near_end ? track.railway_coord_end : track.railway_coord;
        bias_prev = -length(tracks_data1[id_begin].begin_point - point_begin.point);
    }
    // Добавляем первую точку
    branch_track->branch_trajectory.push_back(point_begin);

    for (auto it = branch_track->branch_points.begin(); it != branch_track->branch_points.end(); ++it)
    {
        // Траектория отклонения
        // Отклонение в ZDS длится 100 метров
        // На всякий случай проверяем разбиение стометрового трека на подтреки
        // Ищем трек, который через 100 м - как трек, который хотя бы через 95 м
        size_t id = id_begin;
        double main_traj_l = 0.0;
        do
        {
            ++id;
            main_traj_l = tracks_data1[id].route_coord - coord_begin;
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
            while (tracks_data1[id + 1].route_coord < coord_ref)
            {
                ++id;
            }

            dvec3 main_track_point = tracks_data1[id].begin_point +
                                     tracks_data1[id].orth * (coord_ref - tracks_data1[id].route_coord);

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

        // Завершение траектории
        if ((it+1) == branch_track->branch_points.end())
        {
            branch_track->id_end = id_end;
            // Дописываем последнюю точку
            calculated_branch_point_t point;
            if (branch_track->end_at_other)
            {
                dvec3 point_before_bias = tracks_data1[id_end].begin_point +
                                          tracks_data1[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;
                float coord;
                zds_track_t track = getNearestTrack(point_before_bias, tracks_data2, coord);
                bool near_end = (coord > (track.route_coord + 0.5 * track.length));
                branch_track->id_end_at_other = near_end ? track.prev_uid + 1 : track.prev_uid;

                point.point = near_end ? track.end_point : track.begin_point;
                point.railway_coord = near_end ? track.railway_coord_end : track.railway_coord;
            }
            else
            {
                point.point = tracks_data1[id_end].begin_point +
                              tracks_data2[id_end].right * bias_curr;
                point.railway_coord = tracks_data1[id_end].railway_coord;
            }
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);
            // Добавляем траекторию
            branch_track_data1.push_back(branch_track);
            return true;
        }

        // Расчёт точек траектории, параллельной главному пути, со смещением
        for (id = id_end; id <= (*(it+1))->main_track_id; ++id)
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
        id_begin = (*(it+1))->main_track_id;
        bias_prev = bias_curr;
        bias_curr = (*(it+1))->bias;
        coord_begin = tracks_data1[id_begin].route_coord;

        // Отмечаем разделение траектории светофором, если необходимо
        if ((*(it+1))->id_split_point_by_signal != -1)
            (*(it+1))->id_split_point_by_signal = branch_track->branch_trajectory.size() - 1;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::calcBranchTrack2(zds_branch_track_t* branch_track)
{
    auto branch_point_begin = branch_track->branch_points.begin();
    // Параметры следующего смещения
    size_t id_begin = (*branch_point_begin)->main_track_id;
    double bias_prev = 0.0;
    double bias_curr = (*branch_point_begin)->bias;
    double coord_begin = tracks_data2[id_begin].route_coord + tracks_data2[id_begin].length;

    // Первая точка
    calculated_branch_point_t point_begin;
    point_begin.point = tracks_data2[id_begin].end_point;
    point_begin.trajectory_coord = 0.0;
    point_begin.railway_coord = tracks_data2[id_begin].railway_coord_end;
    branch_track->id_begin = id_begin + 1;
    // Проверяем начало с соседнего пути
    if (branch_track->begin_at_other)
    {
        dvec3 point_after_bias = tracks_data2[id_begin].end_point -
                                 tracks_data2[id_begin].right * bias_curr;
        float coord;
        zds_track_t track = getNearestTrack(point_after_bias, tracks_data1, coord);
        bool near_end = (coord > (track.route_coord + 0.5 * track.length));
        branch_track->id_begin_at_other = near_end ? track.prev_uid + 1 : track.prev_uid;

        point_begin.point = near_end ? track.end_point : track.begin_point;
        point_begin.railway_coord = near_end ? track.railway_coord_end : track.railway_coord;
        bias_prev = -length(tracks_data2[id_begin].end_point - point_begin.point);
    }
    // Добавляем первую точку
    branch_track->branch_trajectory.push_back(point_begin);

    for (auto it = branch_track->branch_points.begin(); it != branch_track->branch_points.end(); ++it)
    {
        // Траектория отклонения
        // Отклонение в ZDS длится 100 метров
        // На всякий случай проверяем разбиение стометрового трека на подтреки
        // Ищем трек, который через 100 м - как трек, который хотя бы через 95 м
        size_t id = id_begin + 1;
        double main_traj_l = 0.0;
        do
        {
            --id;
            main_traj_l = coord_begin - tracks_data2[id].route_coord;
        }
        while (main_traj_l < 95.0);
        size_t id_end = id;
        double railway_coord_length = tracks_data2[id_end].railway_coord - tracks_data2[id_begin].railway_coord_end;

        // Расчёт промежуточных точек по траектории сплайна отклонения
        id = id_begin + 1;
        for (size_t i = 0; i < NUM_BIAS_POINTS; ++i)
        {
            double coord_add = COORD_COEFF[i] * main_traj_l;
            double coord_ref = coord_begin - coord_add;
            // Находим нужный подтрек - у него координата меньше
            while (coord_ref < tracks_data2[id].route_coord)
            {
                --id;
            }

            dvec3 main_track_point = tracks_data2[id].begin_point +
                                     tracks_data2[id].orth * (coord_ref - tracks_data2[id].route_coord);

            // Промежуточная точка отклонения
            calculated_branch_point_t point;
            double bias_coeff = bias_prev * (1.0 - BIAS_COEFF[i]) +
                                bias_curr * BIAS_COEFF[i];
            point.point = main_track_point -
                          tracks_data2[id].right * bias_coeff;
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            point.railway_coord = tracks_data2[id_begin].railway_coord_end +
                                  COORD_COEFF[i] * railway_coord_length;
            // Добавляем промежуточную точку отклонения
            branch_track->branch_trajectory.push_back(point);
        }

        // Завершение траектории
        if ((it+1) == branch_track->branch_points.end())
        {
            branch_track->id_end = id_end;
            // Дописываем последнюю точку
            calculated_branch_point_t point;

            if (branch_track->end_at_other)
            {
                dvec3 point_before_bias = tracks_data2[id_end].begin_point -
                                          tracks_data2[id_end].right * ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK;
                float coord;
                zds_track_t track = getNearestTrack(point_before_bias, tracks_data1, coord);
                bool near_end = (coord > (track.route_coord + 0.5 * track.length));
                branch_track->id_end_at_other = near_end ? track.prev_uid + 1 : track.prev_uid;

                point.point = near_end ? track.end_point : track.begin_point;
                point.railway_coord = near_end ? track.railway_coord_end : track.railway_coord;
            }
            else
            {
                point.point = tracks_data2[id_end].begin_point -
                              tracks_data2[id_end].right * bias_curr;
                point.railway_coord = tracks_data2[id_end].railway_coord;
            }
            double l = length(point.point - branch_track->branch_trajectory.back().point);
            point.trajectory_coord = branch_track->branch_trajectory.back().trajectory_coord + l;
            // Добавляем точку траектории
            branch_track->branch_trajectory.push_back(point);

            // Проверяем, что нет аналогичной траектории с направления "туда"
            for (auto exist_branch : branch_track_data1)
            {
                // Проверяем совпадение точек начала и конца на главных путях
                // (если однопутный участок) или совпадение номера трека
                // начала и конца для отклонений, выделенных из траекторий
                // со съездами через соседний главный путь;
                // если что-то совпало - игнорируем данную траекторию,
                // чтобы не конфликтовать с существующей
                if ((length(tracks_data1[exist_branch->id_begin].begin_point -
                            tracks_data2[branch_track->id_end].begin_point) < 0.1) ||
                    (length(tracks_data1[exist_branch->id_end].begin_point -
                            tracks_data2[branch_track->id_begin].begin_point) < 0.1) ||
                    (exist_branch->id_begin == branch_track->id_end_at_other) ||
                    (exist_branch->id_end == branch_track->id_begin_at_other) ||
                    (exist_branch->id_begin_at_other == branch_track->id_end) ||
                    (exist_branch->id_end_at_other == branch_track->id_begin))
                {
                    // Переносим светофоры из данной траектории в существующую
                    for (auto this_branch_point : branch_track->branch_points)
                    {
                        int point_id = this_branch_point->id_split_point_by_signal;
                        if (point_id == -1)
                            continue;
                        // Проверяем, что в существующей траектории тоже есть такая точка
                        dvec3 point_with_signal = branch_track->branch_trajectory[point_id].point;
                        size_t exist_id = 0;
                        for (auto exist_point = exist_branch->branch_trajectory.begin(); exist_point != exist_branch->branch_trajectory.end(); ++exist_point)
                        {
                            double d = length(exist_point->point - point_with_signal);
                            if (d < 0.1)
                            {
                                zds_branch_point_t* new_point_for_exist = new zds_branch_point_t();
                                new_point_for_exist->main_track_id = this_branch_point->main_track_id;
                                new_point_for_exist->bias = this_branch_point->bias;
                                new_point_for_exist->signal_liter = this_branch_point->signal_liter;
                                new_point_for_exist->signal_special = this_branch_point->signal_special;
                                new_point_for_exist->dir = this_branch_point->dir;
                                new_point_for_exist->id_split_point_by_signal = exist_id;
                                exist_branch->branch_points.push_back(new_point_for_exist);
                                return false;
                            }
                            ++exist_id;
                        }
                    }
                    return false;
                }
            }
            // Добавляем траекторию
            branch_track_data2.push_back(branch_track);
            return true;
        }

        // Расчёт точек траектории, параллельной главному пути, со смещением
        for (id = id_end; id > (*(it+1))->main_track_id; --id)
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
        id_begin = (*(it+1))->main_track_id;
        bias_prev = bias_curr;
        bias_curr = (*(it+1))->bias;
        coord_begin = tracks_data2[id_begin + 1].route_coord;

        // Отмечаем разделение траектории светофором, если необходимо
        if ((*(it+1))->id_split_point_by_signal != -1)
            (*(it+1))->id_split_point_by_signal = branch_track->branch_trajectory.size() - 1;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::calcBranch22(zds_branch_2_2_t* branch22)
{
    // Первая точка
    point_t point_begin;
    point_begin.point = tracks_data1[branch22->id1].begin_point;
    point_begin.railway_coord = tracks_data1[branch22->id1].railway_coord;
    // Добавляем первую точку
    point_begin.trajectory_coord = 0.0;
    branch22->trajectory.points.push_back(point_begin);

    // Последняя точка
    point_t point_end;
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
        point_t point;
        point.point = point_begin.point +
                      mean_orth * COORD_COEFF[i] * coord_length +
                      mean_right * BIAS_COEFF[i] * bias;
        double l = length(point.point - branch22->trajectory.points.back().point);
        point.trajectory_coord = branch22->trajectory.points.back().trajectory_coord + l;
        point.railway_coord = point_begin.railway_coord +
                              COORD_COEFF[i] * railway_coord_length;
        // Добавляем промежуточную точку отклонения
        branch22->trajectory.points.push_back(point);
    }

    // Дописываем последнюю точку
    double l = length(point_end.point - branch22->trajectory.points.back().point);
    point_end.trajectory_coord = branch22->trajectory.points.back().trajectory_coord + l;
    // Добавляем точку траектории
    branch22->trajectory.points.push_back(point_end);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::splitAndNameBranch(zds_branch_track_t* branch_track, const int &dir, size_t num_trajectories)
{
    double trajectory_length = 0.0;
    trajectory_t trajectory = trajectory_t();
    size_t num_sub_traj = 1;
    size_t begin_point_id;
    size_t end_point_id;
    std::string name_prefix;
    std::string name_cur;
    std::string name_next;
    route_connectors_t* split_data;
    route_connectors_t* split_data_other;
    if (dir > 0)
    {
        begin_point_id = 0;
        end_point_id = branch_track->branch_trajectory.size() - 1;
        name_prefix = "branch1_";
        split_data = &split_data1;
        split_data_other = &split_data2;
    }
    else
    {
        begin_point_id = branch_track->branch_trajectory.size() - 1;
        end_point_id = 0;
        name_prefix = "branch2_";
        split_data = &split_data2;
        split_data_other = &split_data1;
    }
    std::string name_suffix = "";
    if (ADD_ZDS_TRACK_NUMBER_TO_FILENAME)
    {
        if (branch_track->begin_at_other)
            name_suffix += QString("_x%1").arg(branch_track->id_begin_at_other + 1).toStdString();
        else
            name_suffix += QString("_%1").arg(branch_track->id_begin + 1).toStdString();

        if (branch_track->end_at_other)
            name_suffix += QString("_x%1").arg(branch_track->id_end_at_other + 1).toStdString();
        else
            name_suffix += QString("_%1").arg(branch_track->id_end + 1).toStdString();
    }
    name_next = name_prefix +
        QString("%1_%2").arg(num_trajectories, 4, 10, QChar('0')).arg(num_sub_traj).toStdString() +
        name_suffix;

    // Добавляем название траектории в коннектор на главном пути
    // Если траектория началась с соседнего главного - добавляем в соседний главный
    if (branch_track->begin_at_other)
    {
        for (auto split = split_data_other->begin(); split != split_data_other->end(); ++split)
        {
            if (dir > 0)
            {
                if ((*split)->track_id == branch_track->id_begin_at_other)
                {
                    (*split)->fwd_side_traj = name_next;
                }
            }
            else
            {
                if ((*split)->track_id == branch_track->id_end_at_other)
                {
                    (*split)->fwd_side_traj = name_next;
                }
            }
        }
    }
    else
    {
        for (auto split = split_data->begin(); split != split_data->end(); ++split)
        {
            if (dir > 0)
            {
                if ((*split)->track_id == branch_track->id_begin)
                {
                    (*split)->fwd_side_traj = name_next;
                }
            }
            else
            {
                if ((*split)->track_id == branch_track->id_end)
                {
                    (*split)->fwd_side_traj = name_next;
                }
            }
        }
    }
    for (size_t i = begin_point_id; i != end_point_id; i += dir)
    {
        point_t point;
        point.point = branch_track->branch_trajectory[i].point;
        point.railway_coord = branch_track->branch_trajectory[i].railway_coord;
        point.trajectory_coord = trajectory_length;
        trajectory.points.push_back(point);

        bool is_split_next = false;
        for (auto it = branch_track->branch_points.begin(); it != branch_track->branch_points.end(); ++it)
        {
            if (((*it)->id_split_point_by_signal != -1) && ((*it)->id_split_point_by_signal == i + dir))
            {
                is_split_next = true;
                name_cur = name_next;
                ++num_sub_traj;
                name_next = name_prefix +
                    QString("%1_%2").arg(num_trajectories, 4, 10, QChar('0')).arg(num_sub_traj).toStdString() +
                    name_suffix;

                split_zds_trajectory_t split;
                split.point = branch_track->branch_trajectory[i + dir].point;
                split.railway_coord = branch_track->branch_trajectory[i + dir].railway_coord;
                split.bwd_main_traj = name_cur;
                split.fwd_main_traj = name_next;
                if ((*it)->dir > 0)
                {
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_FWD);
                    split.signal_fwd_liter = (*it)->signal_liter;
                }
                else
                {
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_BWD);
                    split.signal_bwd_liter = (*it)->signal_liter;
                }
                split.length_bwd_traj = trajectory_length +
                    length(branch_track->branch_trajectory[i + dir].point - trajectory.points.back().point);
                branch_connectors.push_back(new split_zds_trajectory_t(split));
                break;
            }
        }

        if (is_split_next || (i + dir == end_point_id))
        {
            point_t end_point;
            end_point.point = branch_track->branch_trajectory[i + dir].point;
            end_point.railway_coord = branch_track->branch_trajectory[i + dir].railway_coord;
            end_point.trajectory_coord = trajectory_length + length(end_point.point - trajectory.points.back().point);
            trajectory.points.push_back(end_point);
            if (i + dir == end_point_id)
            {
                trajectory.name = name_next;

                // Добавляем название траектории в коннектор на главном пути
                // Если траектория закончилась на соседнем главном - добавляем в соседний главный
                if (branch_track->end_at_other)
                {
                    for (auto split = split_data_other->begin(); split != split_data_other->end(); ++split)
                    {
                        if (dir > 0)
                        {
                            if ((*split)->track_id == branch_track->id_end_at_other)
                            {
                                (*split)->bwd_side_traj = name_next;
                            }
                        }
                        else
                        {
                            if ((*split)->track_id == branch_track->id_begin_at_other)
                            {
                                (*split)->bwd_side_traj = name_next;
                            }
                        }
                    }
                }
                else
                {
                    for (auto split = split_data->begin(); split != split_data->end(); ++split)
                    {
                        if (dir > 0)
                        {
                            if ((*split)->track_id == branch_track->id_end)
                            {
                                (*split)->bwd_side_traj = name_next;
                            }
                        }
                        else
                        {
                            if ((*split)->track_id == branch_track->id_begin)
                            {
                                (*split)->bwd_side_traj = name_next;
                            }
                        }
                    }
                }
            }
            else
            {
                trajectory.name = name_cur;
            }
            branch_track->trajectories.push_back(new trajectory_t(trajectory));

            trajectory.points.clear();
            trajectory_length = 0.0;
        }
        else
        {
            trajectory_length += length(branch_track->branch_trajectory[i + dir].point - trajectory.points.back().point);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::nameBranch22(zds_branch_2_2_t* branch_track, const int &dir, size_t num_trajectories)
{
    std::string name_prefix;
    if (dir > 0)
    {
        name_prefix = "branch2m2_";
    }
    else
    {
        name_prefix = "branch2p2_";

        double trajectory_length = 0.0;
        trajectory_t new_forward_trajectory = trajectory_t();
        for (size_t i = branch_track->trajectory.points.size(); i > 0; --i)
        {
            point_t point = branch_track->trajectory.points[i - 1];
            point.trajectory_coord = trajectory_length;
            new_forward_trajectory.points.push_back(point);

            if (i > 1)
                trajectory_length += length(branch_track->trajectory.points[i - 2].point - new_forward_trajectory.points.back().point);
        }
        branch_track->trajectory = new_forward_trajectory;
    }
    branch_track->trajectory.name = name_prefix +
        QString("%1").arg(num_trajectories, 4, 10, QChar('0')).toStdString();

    if (ADD_ZDS_TRACK_NUMBER_TO_FILENAME)
    {
        if (dir > 0)
            branch_track->trajectory.name +=
                QString("_%1_%2").arg(branch_track->id1 + 1).arg(branch_track->id2).toStdString();
        else
            branch_track->trajectory.name +=
                QString("_%1_%2").arg(branch_track->id1).arg(branch_track->id2 + 1).toStdString();
    }

    for (auto split = split_data1.begin(); split != split_data1.end(); ++split)
    {
        if ((*split)->track_id == branch_track->id1)
        {
            if (dir > 0)
            {
                (*split)->fwd_side_traj = branch_track->trajectory.name;
            }
            else
            {
                (*split)->bwd_side_traj = branch_track->trajectory.name;
            }
        }
    }
    for (auto split = split_data2.begin(); split != split_data2.end(); ++split)
    {
        if ((*split)->track_id == branch_track->id2)
        {
            if (dir > 0)
            {
                (*split)->bwd_side_traj = branch_track->trajectory.name;
            }
            else
            {
                (*split)->fwd_side_traj = branch_track->trajectory.name;
            }
        }
    }
}
