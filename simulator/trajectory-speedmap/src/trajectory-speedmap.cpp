#include    "trajectory-speedmap.h"
#include    "speedmap.h"
#include "topology-connector-device.h"
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
            continue;
        }

        double cur_coord = device.coord;
        size_t cur_idx = 0;
        while (cur_idx < limits.size())
        {
            if ((cur_coord >= limit_begins[cur_idx]) && (cur_coord <= limit_ends[cur_idx]))
            {
                break;
            }
            ++cur_idx;
        }
        if (cur_idx == limits.size())
        {
            device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, 300.0);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_LIMIT, 300.0);
            device.device->setInputSignal(SpeedMap::INPUT_NEXT_DISTANCE, 5000.0);
            continue;
        }

        double dir = device.device->getOutputSignal(SpeedMap::OUTPUT_SEARCH_DIRECTION);
        double cur_distance_coord = cur_coord -
                                    dir * device.device->getOutputSignal(SpeedMap::OUTPUT_CUR_SEARCH_DISTANCE);
        double next_distance_coord = cur_coord +
                                     dir * device.device->getOutputSignal(SpeedMap::OUTPUT_NEXT_SEARCH_DISTANCE);

        size_t next_idx = cur_idx;
        double cur_limit  = limits[cur_idx];
        double next_limit = 300.0;

        // Ищем текущее ограничение - минимальное на длину поезда назад
        TrajectorySpeedMap *cur_traj_device = this;
        cur_coord = dir > 0 ? limit_begins[cur_idx] : limit_ends[cur_idx];
        while (dir * (cur_coord - cur_distance_coord) > 0)
        {
            if (dir > 0)
            {
                if (cur_idx == 0)
                {
                    // Переход к карте скоростей предыдущей траектории
                    auto conn_device = cur_traj_device->getBwdConnectorDevice();
                    if (conn_device == nullptr)
                        break;

                    cur_traj_device = dynamic_cast<TrajectorySpeedMap *>(
                        conn_device->getBwdTrajectoryDevice());
                    if (cur_traj_device == nullptr)
                        break;

                    cur_idx = cur_traj_device->getLimits()->size() - 1;
                    cur_distance_coord += cur_traj_device->getTrajLength();
                }
                else
                {
                    --cur_idx;
                }

                if (cur_limit > cur_traj_device->getLimits()->at(cur_idx))
                {
                    cur_limit = cur_traj_device->getLimits()->at(cur_idx);
                }

                cur_coord = cur_traj_device->getLimitBegins()->at(cur_idx);
            }
            else
            {
                if (cur_idx == cur_traj_device->getLimits()->size() - 1)
                {
                    cur_distance_coord -= cur_traj_device->getTrajLength();

                    // Переход к карте скоростей следующей траектории
                    auto conn_device = cur_traj_device->getFwdConnectorDevice();
                    if (conn_device == nullptr)
                        break;

                    cur_traj_device = dynamic_cast<TrajectorySpeedMap *>(
                        conn_device->getFwdTrajectoryDevice());
                    if (cur_traj_device == nullptr)
                        break;

                    cur_idx = 0;
                }
                else
                {
                    ++cur_idx;
                }

                if (cur_limit > cur_traj_device->getLimits()->at(cur_idx))
                {
                    cur_limit = cur_traj_device->getLimits()->at(cur_idx);
                }

                cur_coord = cur_traj_device->getLimitEnds()->at(cur_idx);
            }
        }

        // Ищем следущее ограничение - минимальное на заданную дистанцию вперёд
        cur_traj_device = this;
        cur_coord = dir > 0 ? limit_ends[next_idx] : limit_begins[next_idx];
        double distance_to_next = dir > 0 ?
                                      limit_ends[next_idx] - device.coord :
                                      device.coord - limit_begins[next_idx];
        double distance_to_min_next = device.device->getOutputSignal(SpeedMap::OUTPUT_NEXT_SEARCH_DISTANCE);
        while (dir * (cur_coord - next_distance_coord) < 0)
        {
            if (dir > 0)
            {
                if (next_idx == cur_traj_device->getLimits()->size() - 1)
                {
                    next_distance_coord -= cur_traj_device->getTrajLength();

                    // Переход к карте скоростей следующей траектории
                    auto conn_device = cur_traj_device->getFwdConnectorDevice();
                    if (conn_device == nullptr)
                        break;

                    cur_traj_device = dynamic_cast<TrajectorySpeedMap *>(
                        conn_device->getFwdTrajectoryDevice());
                    if (cur_traj_device == nullptr)
                        break;

                    next_idx = 0;
                }
                else
                {
                    ++next_idx;
                }

                if (next_limit > cur_traj_device->getLimits()->at(next_idx))
                {
                    next_limit = cur_traj_device->getLimits()->at(next_idx);
                    distance_to_min_next = distance_to_next;
                }

                cur_coord = cur_traj_device->getLimitEnds()->at(next_idx);
                distance_to_next += (cur_coord - next_distance_coord) < 0 ?
                                        (cur_traj_device->getLimitEnds()->at(next_idx) -
                                         cur_traj_device->getLimitBegins()->at(next_idx)) :
                                        (next_distance_coord - cur_traj_device->getLimitBegins()->at(next_idx));

            }
            else
            {
                if (next_idx == 0)
                {
                    // Переход к карте скоростей предыдущей траектории
                    auto conn_device = cur_traj_device->getBwdConnectorDevice();
                    if (conn_device == nullptr)
                        break;

                    cur_traj_device = dynamic_cast<TrajectorySpeedMap *>(
                        conn_device->getBwdTrajectoryDevice());
                    if (cur_traj_device == nullptr)
                        break;

                    next_idx = cur_traj_device->getLimits()->size() - 1;
                    next_distance_coord += cur_traj_device->getTrajLength();
                }
                else
                {

                    --next_idx;
                }

                if (next_limit > cur_traj_device->getLimits()->at(next_idx))
                {
                    next_limit = cur_traj_device->getLimits()->at(next_idx);
                    distance_to_min_next = distance_to_next;
                }

                cur_coord = cur_traj_device->getLimitBegins()->at(next_idx);
                distance_to_next += (cur_coord - next_distance_coord) > 0 ?
                                        (cur_traj_device->getLimitEnds()->at(next_idx) -
                                         cur_traj_device->getLimitBegins()->at(next_idx)) :
                                        (next_distance_coord - cur_traj_device->getLimitBegins()->at(next_idx));
            }
        }

        device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, cur_limit);
        device.device->setInputSignal(SpeedMap::INPUT_NEXT_LIMIT, next_limit);
        device.device->setInputSignal(SpeedMap::INPUT_NEXT_DISTANCE, distance_to_min_next);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<double> *TrajectorySpeedMap::getLimits()
{
    return &limits;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<double> *TrajectorySpeedMap::getLimitBegins()
{
    return &limit_begins;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<double> *TrajectorySpeedMap::getLimitEnds()
{
    return &limit_ends;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TrajectorySpeedMap::getTrajLength()
{
    return trajectory->getLength();
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
