#include    "trajectory-speedmap.h"
#include    "speedmap.h"
#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectorySpeedMap::TrajectorySpeedMap(QObject *parent) : TrajectoryDevice(parent)
{
    name = QString("speedmap");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectorySpeedMap::~TrajectorySpeedMap()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectorySpeedMap::step(double t, double dt)
{
    (void) t;
    (void) dt;

    for (auto device : vehicles_devices)
    {
        if (limits.empty())
        {
            device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, 300.0);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_LIMIT, 300.0);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_DISTANCE, 5000.0);
        }
        else
        {
            double cur_coord = device.coord;
            double dir = device.device->getOutputSignal(SpeedMap::OUTPUT_SEARCH_DIRECTION);
            double cur_distance = cur_coord -
                dir * device.device->getOutputSignal(SpeedMap::OUTPUT_CUR_SEARCH_DISTANCE);
            double next_distance = cur_coord +
                dir * device.device->getOutputSignal(SpeedMap::OUTPUT_NEXT_SEARCH_DISTANCE);

            double cur_limit = 0.0;
            double next_limit = 0.0;
            size_t cur_idx = 0;
            while (cur_idx < limits.size())
            {
                if ((cur_coord >= limit_begins[cur_idx]) && (cur_coord <= limit_ends[cur_idx]))
                {
                    cur_limit = limits[cur_idx];
                    break;
                }
            }

            device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, cur_limit);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_LIMIT, next_limit);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_DISTANCE, 5000.0);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectorySpeedMap::load_config(CfgReader &cfg)
{
    // Проверяем все карты скоростей в конфиге,
    // пока не найдём ту, в которой есть имя данной траектории
    QDomNode speedmap_node = cfg.getFirstSection("SpeedMap");
    while (!speedmap_node.isNull())
    {
        QDomNodeList nodes = speedmap_node.childNodes();
        // Проверяем все имена траекторий в карте скоростей,
        // пока не найдём имя данной траектории
        bool is_speedmap = false;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            QString node_name = nodes.at(i).nodeName();
            if (node_name == "Trajectory")
            {
                QString traj_name = nodes.at(i).toElement().text();
                if (traj_name == trajectory->getName())
                {
                    is_speedmap = true;
                    break;
                }
            }
        }

        if (is_speedmap)
        {
            // Треки данной траектории
            std::vector<track_t> tracks = trajectory->getTracks();

            // Интервалы пикетажа данной траектории
            double railway_coord_begin = min(tracks.front().railway_coord0,
                                             tracks.back().railway_coord1);
            double railway_coord_end = max(tracks.front().railway_coord0,
                                           tracks.back().railway_coord1);

            // Проверяем все ограничения скорости в карте по интервалам пикетажа
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                QString node_name = nodes.at(i).nodeName();
                if (node_name == "SpeedLimit")
                {
                    QString value = nodes.at(i).toElement().text();
                    QStringList tokens = value.split(' ');
                    double limit = tokens[0].toDouble();
                    double limit_begin = tokens.size() > 1 ? tokens[1].toDouble() : -1.0;
                    double limit_end = tokens.size() > 2 ? tokens[2].toDouble() : 1000000000.0;

                    // Если интервалы пикетажа пересекаются - добавляем ограничение в модуль
                    if ((limit_end > railway_coord_begin) && (limit_begin < railway_coord_end))
                    {
                        limits.push_back(limit);
                        double traj_limit_begin = 0.0;
                        double traj_limit_end = trajectory->getLength();
                        for (auto track : tracks)
                        {
                            double track_begin = min(track.railway_coord0,
                                                     track.railway_coord1);
                            double track_end = max(track.railway_coord0,
                                                   track.railway_coord1);

                            if ((limit_begin >= track_begin) &&
                                (limit_begin < track_end))
                            {
                                double relative_coord = (limit_begin - track_begin) / (track_end - track_begin);
                                traj_limit_begin = track.traj_coord + relative_coord * track.len;
                            }

                            if ((limit_end >= track_begin) &&
                                (limit_end < track_end))
                            {
                                double relative_coord = (limit_end - track_begin) / (track_end - track_begin);
                                traj_limit_end = track.traj_coord + relative_coord * track.len;
                            }
                        }
                        limit_begins.push_back(traj_limit_begin);
                        limit_ends.push_back(traj_limit_end);
                    }
                }
            }
        }

        // Переходим к следующей карте скоростей
        speedmap_node = cfg.getNextSection();
    }
}

GET_TRAJECTORY_DEVICE(TrajectorySpeedMap)
