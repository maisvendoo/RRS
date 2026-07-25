#include    "trajectory-speedmap.h"
#include    "speedmap.h"
#include    "topology-connector-device.h"
#include    "trajectory.h"
#include    <core/get_module.h>

#include    "physics.h"

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

    size_t device_idx = 0;
    for (auto device : vehicles_devices)
    {
        std::int8_t search_dir = vehicles_devices_directions[device_idx];
        ++device_idx;

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


        // Вспомогательная функция для перемещения по карте скоростей
        auto next_traj_device = [](TrajectorySpeedMap*& _cur_traj_device,
                                   std::int8_t& _dir,
                                   size_t& _cur_idx,
                                   double& _cur_coord) -> bool
        {
            if (_dir > 0)
            {
                _cur_coord -= _cur_traj_device->getTrajLength();
            }

            // Переход к карте скоростей следующей траектории
            auto conn_device = _cur_traj_device->getNextConnectorDevice(_dir);
            if (conn_device == nullptr)
            {
                return false;
            }

            _cur_traj_device = dynamic_cast<TrajectorySpeedMap *>(
                conn_device->getNextTrajectoryDevice(_dir));
            if (_cur_traj_device == nullptr)
            {
                return false;
            }

            _cur_idx = _cur_traj_device->getLimits()->size();
            if (_cur_idx == 0)
            {
                return false;
            }

            if (_dir > 0)
            {
                _cur_idx = 0;
            }
            else
            {
                --_cur_idx;
                _cur_coord += _cur_traj_device->getTrajLength();
            }
            return true;
        };

        // Вспомогательная функция для сохранения минимального из ограничений
        auto save_minimum_limit = [](double& _cur_limit,
                                     const std::vector<double>& _limits, const size_t& _cur_idx)
        {
            const double _new_limit = _limits[_cur_idx];
            if (_cur_limit > _new_limit)
                _cur_limit = _new_limit;
        };

        // Вспомогательная функция для сохранения следующего ближайшего ограничения
        auto save_next_limit = [](bool& _first, double& _next_limit, double& _distance_to_next_limit,
                                  const std::vector<double>& _limits, const size_t& _next_idx,
                                  const double& _distance, const double _brake_acceleration) -> bool
        {
            const double _new_limit = _limits[_next_idx];
            // Ближайшее (_first==true) ограничение сохраняем безусловно
            if (_first)
            {
                if (_new_limit == _next_limit)
                {
                    return false;
                }
                _first = false;
                _next_limit = _new_limit;
                _distance_to_next_limit = _distance;
                return false;
            }

            if (_brake_acceleration > Physics::ZERO)
            {
                // Тормозной путь от V1 до V2: d = (V1 ^ 2 - V2 ^ 2) / (2 * a)
                constexpr double _denominator_coeff = 2.0 * Physics::kmh * Physics::kmh;
                const double _denominator = _denominator_coeff * _brake_acceleration;
                double _brake_distance = _next_limit * _next_limit / _denominator;

                // Если уже прошли тормозной путь полностью, возращаем true,
                // как признак, что просматривать дальше нет смысла
                if (_brake_distance > (_distance - _distance_to_next_limit))
                {
                    return true;
                }

                // Последующие ограничения сохраняем вместо ближайшего,
                // только если они более строгие: меньше и находятся ближе,
                // чем требуемый тормозной путь
                if (_new_limit < _next_limit)
                {
                    _brake_distance = (_next_limit * _next_limit - _new_limit * _new_limit) / (_denominator_coeff * _brake_acceleration);
                    if (_brake_distance > (_distance - _distance_to_next_limit))
                    {
                        _next_limit = _new_limit;
                        _distance_to_next_limit = _distance;
                    }
                }
            }
            else
            {
                // Если тормозное ускорение не задано, сохраняем наименьшее ограничение
                if (_new_limit < _next_limit)
                {
                    _next_limit = _new_limit;
                    _distance_to_next_limit = _distance;
                }
            }
            return false;
        };

        TrajectorySpeedMap* cur_traj_device = this;
        size_t next_idx = cur_idx;

        if (device.device->getOutputSignal(SpeedMap::OUTPUT_SEARCH_DIRECTION) < Physics::ZERO)
        {
            search_dir = -search_dir;
        }
        const double cur_search_distance = device.device->getOutputSignal(SpeedMap::OUTPUT_CUR_SEARCH_DISTANCE);
        const double next_search_distance = device.device->getOutputSignal(SpeedMap::OUTPUT_NEXT_SEARCH_DISTANCE);
        double cur_limit = limits[cur_idx];
        double next_limit = limits[next_idx];

        // Ищем текущее ограничение - минимальное на длину поезда назад
        std::int8_t dir = (search_dir > 0) ? BWD : FWD;
        double distance = (dir > 0) ?
                              limit_ends[cur_idx] - device.coord :
                              device.coord - limit_begins[cur_idx];
        cur_coord = (dir > 0) ?
                        limit_ends[cur_idx] :
                        limit_begins[cur_idx];
        while (distance < cur_search_distance)
        {
            if (dir > 0)
            {
                if (cur_idx == cur_traj_device->getLimits()->size() - 1)
                {
                    if (!next_traj_device(cur_traj_device, dir, cur_idx, cur_coord))
                    {
                        break;
                    }
                }
                else
                {
                    ++cur_idx;
                }
            }
            else
            {
                if (cur_idx == 0)
                {
                    if (!next_traj_device(cur_traj_device, dir, cur_idx, cur_coord))
                    {
                        break;
                    }
                }
                else
                {
                    --cur_idx;
                }
            }

            if (dir > 0)
            {
                distance += cur_traj_device->getLimitEnds()->at(cur_idx) - cur_coord;
                cur_coord = cur_traj_device->getLimitEnds()->at(cur_idx);
            }
            else
            {
                distance += cur_coord - cur_traj_device->getLimitBegins()->at(cur_idx);
                cur_coord = cur_traj_device->getLimitBegins()->at(cur_idx);
            }

            save_minimum_limit(cur_limit, (*cur_traj_device->getLimits()), cur_idx);
        }

        // Ищем следущее ограничение - минимальное на заданную дистанцию вперёд
        cur_traj_device = this;
        dir = (search_dir > 0) ? FWD : BWD;
        distance = (dir > 0) ?
                       limit_ends[next_idx] - device.coord :
                       device.coord - limit_begins[next_idx];
        cur_coord = (dir > 0) ?
                        limit_ends[next_idx] :
                        limit_begins[next_idx];
        bool is_first = true;
        double distance_to_next_limit = next_search_distance;
        while (distance < next_search_distance)
        {
            if (dir > 0)
            {
                if (next_idx == cur_traj_device->getLimits()->size() - 1)
                {
                    if (!next_traj_device(cur_traj_device, dir, next_idx, cur_coord))
                    {
                        break;
                    }
                }
                else
                {
                    ++next_idx;
                }
            }
            else
            {
                if (next_idx == 0)
                {
                    if (!next_traj_device(cur_traj_device, dir, next_idx, cur_coord))
                    {
                        break;
                    }
                }
                else
                {
                    --next_idx;
                }
            }

            if (save_next_limit(is_first, next_limit, distance_to_next_limit,
                                (*cur_traj_device->getLimits()), next_idx,
                                distance, device.device->getOutputSignal(SpeedMap::OUTPUT_BRAKE_ACCELERATION)))
            {
                break;
            }

            if (dir > 0)
            {
                distance += cur_traj_device->getLimitEnds()->at(next_idx) - cur_coord;
                cur_coord = cur_traj_device->getLimitEnds()->at(next_idx);
            }
            else
            {
                distance += cur_coord - cur_traj_device->getLimitBegins()->at(next_idx);
                cur_coord = cur_traj_device->getLimitBegins()->at(next_idx);
            }
        }

        device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, cur_limit);
        device.device->setInputSignal(SpeedMap::INPUT_NEXT_LIMIT, next_limit);
        device.device->setInputSignal(SpeedMap::INPUT_NEXT_DISTANCE, distance_to_next_limit);
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
    // Треки данной траектории
    const std::vector<track_t>& tracks = trajectory->getTracks();

    // Интервалы пикетажа данной траектории
    double railway_coord_begin = min(tracks.front().railway_coord0,
                                     tracks.back().railway_coord1);
    double railway_coord_end = max(tracks.front().railway_coord0,
                                   tracks.back().railway_coord1);

    QDomNode speedmap_node = cfg.getFirstSection("SpeedMap");
    while (!speedmap_node.isNull())
    {
        QString speed_limit;
        cfg.getString(speedmap_node, "SpeedLimit",speed_limit);
        QStringList tokens = speed_limit.split(' ');
        double limit = tokens[0].toDouble();
        double limit_coord_1 = tokens.size() > 1 ? tokens[1].toDouble() : -1.0;
        double limit_coord_2 = tokens.size() > 2 ? tokens[2].toDouble() : 1000000000.0;
        double limit_begin = min(limit_coord_1, limit_coord_2);
        double limit_end = max(limit_coord_1, limit_coord_2);

        // Если интервал пикетажа пересекается с пикетажем треков траектории,
        // добавляем ограничение в модуль
        if ((limit_end > railway_coord_begin) && (limit_begin < railway_coord_end))
        {
            limits.push_back(limit);
            double traj_limit_begin = 0.0;
            double traj_limit_end = trajectory->getLength();
            for (const auto& track : tracks)
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

        // Переходим к следующей карте скоростей
        speedmap_node = cfg.getNextSection();
    }
}

GET_MODULE(TrajectorySpeedMap)
