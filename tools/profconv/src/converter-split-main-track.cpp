#include    "converter.h"

#include    <QFile>

#include    "path-utils.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::findSplitsMainTrajectories(const int &dir)
{
    if (dir > 0)
    {
        bool was_1_track = false;
        size_t id = 0;
        for (auto it = tracks_data1.begin(); it != tracks_data1.end(); ++it)
        {
            zds_track_t track = *it;
            split_zds_trajectory_t split = split_zds_trajectory_t();

            // Проверяем однопутный участок
            for (auto it2 = tracks_data2.begin(); it2 != tracks_data2.end(); ++it2)
            {
                zds_track_t track2 = *it2;
                if (length(track.begin_point - track2.begin_point) < 0.1)
                {
                    if ((!was_1_track) && (id != 0))
                    {
                        // Начало однопутного участка
                        split.split_type.push_back(split_zds_trajectory_t::SPLIT_2_1);
                    }

                    if (length(track.end_point - track2.end_point) < 0.1)
                    {
                        // Однопутный участок
                        it2->id_at_track1 = id;
                        was_1_track = true;
                    }
                    else
                    {
                        // Конец однопутного участка
                        split.split_type.push_back(split_zds_trajectory_t::SPLIT_1_2);
                        was_1_track = false;
                    }
                    break;
                }
            }

            // Съезды между главными путями
            // Добавляем съезды "2-2", найденные в branch_tracks
            bool is_branch = false;
            for (auto branch22 : branch_2minus2_data)
            {
                if (branch22->id1 == id)
                {
                    is_branch = true;

                    // Разделение в начале данного трека
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);

                    // Добавляем разделение и в соседний главный путь
                    split_zds_trajectory_t split2 = split_zds_trajectory_t();
                    split2.track_id = branch22->id2;
                    split2.point = tracks_data2[branch22->id2].begin_point;
                    split2.railway_coord = tracks_data2[branch22->id2].railway_coord;
                    split2.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);
                    addOrCreateSplit(split_data2, split2);

                    break;
                }
            }
            // Проверяем съезды "2-2", записанные в route.trk, игнорируем однопутные участки
            if ((track.arrows == "2-2") && (!is_branch) && (!was_1_track))
            {
                // Разделение в начале данного трека
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);

                // Добавляем разделение и в соседний главный путь
                float coord;
                zds_track_t track2 = getNearestTrack(track.end_point, tracks_data2, coord);
                bool near_end = (coord > (track2.route_coord + 0.5 * track2.length));
                size_t id2 = near_end ? track2.prev_uid + 1 : track2.prev_uid;

                split_zds_trajectory_t split2 = split_zds_trajectory_t();
                split2.track_id = id2;
                split2.point = tracks_data2[id2].begin_point;
                split2.railway_coord = tracks_data2[id2].railway_coord;
                split2.split_type.push_back(split_zds_trajectory_t::SPLIT_2MINUS2);
                addOrCreateSplit(split_data2, split2);

                // Добавляем съезд в массив траекторий съездов
                zds_branch_2_2_t branch2minus2 = zds_branch_2_2_t();
                branch2minus2.id1 = id;
                branch2minus2.id2 = id2;
                calcBranch22(&branch2minus2);
                branch_2minus2_data.push_back(new zds_branch_2_2_t(branch2minus2));
            }
            // Добавляем съезды "2+2", найденные в branch_tracks
            is_branch = false;
            for (auto branch22 : branch_2plus2_data)
            {
                if (branch22->id1 == id + 1)
                {
                    is_branch = true;

                    // Разделение в конце данного трека - в начале следующего
                    split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                    split_at_next.track_id = id + 1;
                    split_at_next.point = track.end_point;
                    split_at_next.railway_coord = track.railway_coord_end;
                    split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                    // Проверяем светофор на возвращении с branch_track
                    if ((branch22->is_bwd) && (!branch22->branch_point_bwd.signal_liter.empty()))
                    {
                        split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_BWD);
                        split_at_next.signal_bwd_liter = branch22->branch_point_bwd.signal_liter;
                    }
                    addOrCreateSplit(split_data1, split_at_next);

                    // Добавляем разделение и в соседний главный путь
                    split_zds_trajectory_t split2 = split_zds_trajectory_t();
                    split2.track_id = branch22->id2;
                    split2.point = tracks_data2[branch22->id2].begin_point;
                    split2.railway_coord = tracks_data2[branch22->id2].railway_coord;
                    split2.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                    // Проверяем светофор на возвращении с branch_track
                    if ((branch22->is_fwd) && (!branch22->branch_point_fwd.signal_liter.empty()))
                    {
                        split2.split_type.push_back(split_zds_trajectory_t::SPLIT_SIGNAL_FWD);
                        split2.signal_fwd_liter = branch22->branch_point_fwd.signal_liter;
                    }
                    addOrCreateSplit(split_data2, split2);

                    break;
                }
            }
            // Проверяем съезды "2+2", записанные в route.trk, игнорируем однопутные участки
            if ((track.arrows == "2+2") && (!is_branch) && (!was_1_track))
            {
                // Разделение в конце данного трека - в начале следующего
                split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                split_at_next.track_id = id + 1;
                split_at_next.point = track.end_point;
                split_at_next.railway_coord = track.railway_coord_end;
                split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                addOrCreateSplit(split_data1, split_at_next);

                // Добавляем разделение и в соседний главный путь
                float coord;
                zds_track_t track2 = getNearestTrack(track.begin_point, tracks_data2, coord);
                bool near_end = (coord > (track2.route_coord + 0.5 * track2.length));
                size_t id2 = near_end ? track2.prev_uid + 1 : track2.prev_uid;

                split_zds_trajectory_t split2 = split_zds_trajectory_t();
                split2.track_id = id2;
                split2.point = tracks_data2[id2].begin_point;
                split2.railway_coord = tracks_data2[id2].railway_coord;
                split2.split_type.push_back(split_zds_trajectory_t::SPLIT_2PLUS2);
                addOrCreateSplit(split_data2, split2);

                // Добавляем съезд в массив траекторий съездов
                zds_branch_2_2_t branch2plus2 = zds_branch_2_2_t();
                branch2plus2.id1 = id + 1;
                branch2plus2.id2 = id2;
                calcBranch22(&branch2plus2);
                branch_2plus2_data.push_back(new zds_branch_2_2_t(branch2plus2));
            }

            // Стрелки на боковые пути
            // Добавляем съезды на боковые пути, найденные в branch_tracks
            is_branch = false;
            for (auto branch : branch_track_data1)
            {
                if ((branch->id_begin == id) && (!branch->begin_at_other))
                {
                    is_branch = true;
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE);
                }
            }
            for (auto branch : branch_track_data2)
            {
                if (branch->id_begin_at_other == id)
                {
                    is_branch = true;
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE);
                }
            }
            // Добавляем стрелки "+2" или "-2", записанные в route.trk
            if ( ((track.arrows == "+2") || (track.arrows == "-2")) && (!is_branch) )
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE_NO_BRANCH);
            }
            // Добавляем съезды с боковых путей, найденных в branch_tracks
            is_branch = false;
            for (auto branch : branch_track_data1)
            {
                if ((branch->id_end == id + 1) && (!branch->end_at_other))
                {
                    is_branch = true;

                    // Разделение в конце данного трека - в начале следующего
                    split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                    split_at_next.track_id = id + 1;
                    split_at_next.point = track.end_point;
                    split_at_next.railway_coord = track.railway_coord_end;
                    split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE);
                    addOrCreateSplit(split_data1, split_at_next);
                }
            }
            for (auto branch : branch_track_data2)
            {
                if (branch->id_end_at_other == id + 1)
                {
                    is_branch = true;

                    // Разделение в конце данного трека - в начале следующего
                    split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                    split_at_next.track_id = id + 1;
                    split_at_next.point = track.end_point;
                    split_at_next.railway_coord = track.railway_coord_end;
                    split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE);
                    addOrCreateSplit(split_data1, split_at_next);
                }
            }
            // Добавляем стрелки "2+" или "2-", записанные в route.trk
            if ( ((track.arrows == "2+") || (track.arrows == "2-")) && (!is_branch) )
            {
                // Разделение в конце данного трека - в начале следующего
                split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                split_at_next.track_id = id + 1;
                split_at_next.point = track.end_point;
                split_at_next.railway_coord = track.railway_coord_end;
                split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE_NO_BRANCH);
                addOrCreateSplit(split_data1, split_at_next);
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
                addOrCreateSplit(split_data1, split);
            }

            ++id;
        }
    }
    else
    {
        size_t id = 0;
        for (auto it = tracks_data2.begin(); it != tracks_data2.end(); ++it)
        {
            zds_track_t track = *it;
            split_zds_trajectory_t split = split_zds_trajectory_t();

            // Стрелки на боковые пути
            // Добавляем съезды на боковые пути, найденные в branch_tracks
            bool is_branch = false;
            for (auto branch : branch_track_data2)
            {
                if ((branch->id_end == id) && (!branch->end_at_other))
                {
                    is_branch = true;
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE);
                }
            }
            for (auto branch : branch_track_data1)
            {
                if (branch->id_end_at_other == id)
                {
                    is_branch = true;
                    split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE);
                }
            }
            // Добавляем стрелки "+2" или "-2", записанные в route.trk
            if ( ((track.arrows == "+2") || (track.arrows == "-2")) && (!is_branch) )
            {
                split.split_type.push_back(split_zds_trajectory_t::SPLIT_TO_SIDE_NO_BRANCH);
            }
            // Добавляем съезды с боковых путей, найденных в branch_tracks
            is_branch = false;
            for (auto branch : branch_track_data2)
            {
                if ((branch->id_begin == id + 1) && (!branch->begin_at_other))
                {
                    is_branch = true;

                    // Разделение в конце данного трека - в начале следующего
                    split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                    split_at_next.track_id = id + 1;
                    split_at_next.point = track.end_point;
                    split_at_next.railway_coord = track.railway_coord_end;
                    split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE);
                    if (track.id_at_track1 == -1)
                    {
                        addOrCreateSplit(split_data2, split_at_next);
                    }
                    else
                    {
                        split_at_next.track_id = track.id_at_track1;
                        addOrCreateSplit(split_data1, split_at_next);
                    }
                }
            }
            for (auto branch : branch_track_data1)
            {
                if (branch->id_begin_at_other == id + 1)
                {
                    is_branch = true;

                    // Разделение в конце данного трека - в начале следующего
                    split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                    split_at_next.track_id = id + 1;
                    split_at_next.point = track.end_point;
                    split_at_next.railway_coord = track.railway_coord_end;
                    split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE);
                    if (track.id_at_track1 == -1)
                    {
                        addOrCreateSplit(split_data2, split_at_next);
                    }
                    else
                    {
                        split_at_next.track_id = track.id_at_track1;
                        addOrCreateSplit(split_data1, split_at_next);
                    }
                }
            }
            // Добавляем стрелки "2+" или "2-", записанные в route.trk
            if ( ((track.arrows == "2+") || (track.arrows == "2-")) && (!is_branch))
            {
                // Разделение в конце данного трека - в начале следующего
                split_zds_trajectory_t split_at_next = split_zds_trajectory_t();
                split_at_next.track_id = id + 1;
                split_at_next.point = track.end_point;
                split_at_next.railway_coord = track.railway_coord_end;
                split_at_next.split_type.push_back(split_zds_trajectory_t::SPLIT_FROM_SIDE_NO_BRANCH);
                if (track.id_at_track1 == -1)
                {
                    addOrCreateSplit(split_data2, split_at_next);
                }
                else
                {
                    split_at_next.track_id = track.id_at_track1;
                    addOrCreateSplit(split_data1, split_at_next);
                }
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

            if (!split.split_type.empty())
            {
                split.track_id = id;
                split.point = track.begin_point;
                split.railway_coord = track.railway_coord;
                if (track.id_at_track1 == -1)
                {
                    addOrCreateSplit(split_data2, split);
                }
                else
                {
                    split.track_id = track.id_at_track1;
                    addOrCreateSplit(split_data1, split);
                }
            }

            ++id;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::addOrCreateSplit(route_connectors_t &split_data, const split_zds_trajectory_t &split_point)
{
    if (split_point.split_type.empty())
        return;

    auto exist_it = split_data.begin();
    while (exist_it != split_data.end())
    {
        if ((*exist_it)->track_id == split_point.track_id)
        {
            for (auto type_it : split_point.split_type)
            {
                (*exist_it)->split_type.push_back(type_it);
                if (type_it == split_zds_trajectory_t::SPLIT_SIGNAL_FWD)
                {
                    (*exist_it)->signal_bwd_type = split_point.signal_fwd_type;
                    (*exist_it)->signal_bwd_liter = split_point.signal_fwd_liter;
                }
                if (type_it == split_zds_trajectory_t::SPLIT_SIGNAL_BWD)
                {
                    (*exist_it)->signal_bwd_type = split_point.signal_bwd_type;
                    (*exist_it)->signal_bwd_liter = split_point.signal_bwd_liter;
                }
            }
            return;
        }

        ++exist_it;
    }

    split_data.push_back(new split_zds_trajectory_t(split_point));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::splitMainTrajectory(const int &dir)
{
    double trajectory_length = 0.0;
    trajectory_t trajectory = trajectory_t();
    size_t num_traj = 1;
    std::string name_prefix;
    std::string name_cur;
    std::string name_next;
    route_trajectories_t* trajectories;
    zds_trajectory_data_t* tracks_data;
    route_connectors_t* split_data;
    if (dir > 0)
    {
        name_prefix = "route1_";
        split_data = &split_data1;
        tracks_data = &tracks_data1;
        trajectories = &trajectories1;
    }
    else
    {
        name_prefix = "route2_";
        split_data = &split_data2;
        tracks_data = &tracks_data2;
        trajectories = &trajectories2;
    }

    name_next = name_prefix +
                QString("%1").arg(num_traj, 4, 10, QChar('0')).toStdString();
    if (ADD_ZDS_TRACK_NUMBER_TO_FILENAME)
        name_next += "_1";

    bool was_1_track = false;
    for (auto it = tracks_data->begin(); it != tracks_data->end(); ++it)
    {
        if ((dir < 0) && (it->id_at_track1 != -1))
        {
            if ((!was_1_track) && (it->id_at_track1 != 0))
            {
                // Начало однопутного участка
                for (auto split = split_data1.begin(); split != split_data1.end(); ++split)
                {
                    if ((*split)->track_id == it->id_at_track1)
                    {
                        name_cur = name_next;
                        ++num_traj;
                        name_next = name_prefix +
                                    QString("%1").arg(num_traj, 4, 10, QChar('0')).toStdString();
                        if (ADD_ZDS_TRACK_NUMBER_TO_FILENAME)
                        {
                            name_next += QString("_%1").arg(it->prev_uid + 2).toStdString();
                        }

                        // Добавляем последюю точку в траекторию
                        point_t end_point;
                        end_point.point = it->begin_point;
                        end_point.railway_coord = it->railway_coord;
                        end_point.trajectory_coord = trajectory_length;
                        trajectory.points.push_back(end_point);
                        trajectory.name = name_cur;
                        trajectories->push_back(new trajectory_t(trajectory));

                        // Делаем траекторию главного пути "туда" сзади боковой
                        (*split)->bwd_side_traj = (*split)->bwd_main_traj;
                        // Делаем траекторию данного пути "обратно" сзади основной
                        (*split)->bwd_main_traj = name_cur;
                        (*split)->length_bwd_traj = trajectory_length;

                        trajectory.points.clear();
                        trajectory_length = 0.0;

                    }
                }
            }
            if ((it + 1) == tracks_data->end())
                return;

            if ((it + 1)->id_at_track1 != -1)
            {
                // Однопутный участок
                was_1_track = true;
                it->trajectory_name = tracks_data1[it->id_at_track1].trajectory_name;
                it->trajectory_coord = tracks_data1[it->id_at_track1].trajectory_coord;
            }
            else
            {
                // Конец однопутного участка
                if (was_1_track)
                {
                    was_1_track = false;
                    for (auto split = split_data1.begin(); split != split_data1.end(); ++split)
                    {
                        if ((*split)->track_id == ((it)->id_at_track1 + 1))
                        {
                            // Делаем траекторию данного пути "обратно" спереди боковой
                            (*split)->fwd_side_traj = name_next;
                        }
                    }
                }
            }
            continue;
        }

        point_t point;
        point.point = it->begin_point;
        point.railway_coord = it->railway_coord;
        point.trajectory_coord = trajectory_length;
        trajectory.points.push_back(point);
        it->trajectory_name = name_next;
        it->trajectory_coord = trajectory_length;

        size_t id = it->prev_uid;
        bool is_split_next = false;
        for (auto split = split_data->begin(); split != split_data->end(); ++split)
        {
            if ((*split)->track_id == id + 1)
            {
                is_split_next = true;
                name_cur = name_next;
                ++num_traj;
                name_next = name_prefix +
                    QString("%1").arg(num_traj, 4, 10, QChar('0')).toStdString();
                if (ADD_ZDS_TRACK_NUMBER_TO_FILENAME)
                {
                    name_next += QString("_%1").arg(it->prev_uid + 2).toStdString();
                }
                (*split)->bwd_main_traj = name_cur;
                (*split)->fwd_main_traj = name_next;
                (*split)->length_bwd_traj = trajectory_length + it->length;
            }
        }
        if (is_split_next || ((it+1) == tracks_data->end()))
        {
            point_t end_point;
            end_point.point = it->end_point;
            end_point.railway_coord = it->railway_coord_end;
            end_point.trajectory_coord = trajectory_length + it->length;
            trajectory.points.push_back(end_point);
            trajectory.name = ((it+1) == tracks_data->end()) ? name_next : name_cur;
            trajectories->push_back(new trajectory_t(trajectory));

            trajectory.points.clear();
            trajectory_length = 0.0;
        }
        else
        {
            trajectory_length += it->length;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::writeSplits(const route_connectors_t &connectors, const int &dir)
{
    // Вывод в файл, для отладки
    std::string path;
    if (dir == 0)
    {
        path = compinePath(toNativeSeparators(topologyDir), "branch_split.conf");
    }
    if (dir == 1)
    {
        path = compinePath(toNativeSeparators(topologyDir), "split1.conf");
    }
    if (dir == -1)
    {
        path = compinePath(toNativeSeparators(topologyDir), "split2.conf");
    }

    QFile file_old(QString(path.c_str()));
    if (file_old.exists())
        file_old.rename( QString((path + FILE_BACKUP_EXTENTION).c_str()) );

    QFile file(QString(path.c_str()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    for (auto it = connectors.begin(); it != connectors.end(); ++it)
    {
        stream                  << (*it)->point.x
            << DELIMITER_SYMBOL << (*it)->point.y
            << DELIMITER_SYMBOL << (*it)->point.z
            << DELIMITER_SYMBOL << (*it)->track_id
            << DELIMITER_SYMBOL << static_cast<int>(round((*it)->railway_coord))
            << DELIMITER_SYMBOL << (*it)->bwd_main_traj.c_str()
            << DELIMITER_SYMBOL << (*it)->fwd_main_traj.c_str();
        for (auto type_it : (*it)->split_type)
        {
            switch (type_it)
            {
            case split_zds_trajectory_t::SPLIT_2MINUS2:
            {
                stream << DELIMITER_SYMBOL
                       << "splitby2minus2_";
                if (dir == 1)
                    stream << (*it)->fwd_side_traj.c_str();
                if (dir == -1)
                    stream << (*it)->bwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_2PLUS2:
            {
                stream << DELIMITER_SYMBOL
                       << "splitby2plus2_";
                if (dir == 1)
                    stream << (*it)->bwd_side_traj.c_str();
                if (dir == -1)
                    stream << (*it)->fwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_2_1:
            {
                stream << DELIMITER_SYMBOL
                       << "splitby1begin_" << (*it)->bwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_1_2:
            {
                stream << DELIMITER_SYMBOL
                       << "splitby2begin_" << (*it)->fwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_TO_SIDE:
            {
                stream << DELIMITER_SYMBOL
                       << "splitbyside2_" << (*it)->fwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_FROM_SIDE:
            {
                stream << DELIMITER_SYMBOL
                       << "splitbyside1_" << (*it)->bwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_TO_SIDE_NO_BRANCH:
            {
                stream << DELIMITER_SYMBOL
                       << "splitbyside2_!!_NO_BRANCH_!!_" << (*it)->fwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_FROM_SIDE_NO_BRANCH:
            {
                stream << DELIMITER_SYMBOL
                       << "splitbyside1_!!_NO_BRANCH_!!_" << (*it)->bwd_side_traj.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_SIGNAL_FWD:
            {
                stream << DELIMITER_SYMBOL
                       << "splitsignF________________" << (*it)->signal_fwd_liter.c_str();
                break;
            }
            case split_zds_trajectory_t::SPLIT_SIGNAL_BWD:
            {
                stream << DELIMITER_SYMBOL
                       << "splitsignB________________" << (*it)->signal_bwd_liter.c_str();
                break;
            }
            default: stream << DELIMITER_SYMBOL << "WARN_" << type_it;
            }
        }
        stream << "\n";
    }

    file.close();
}
