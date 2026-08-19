#include    "trajectory.h"

#include    "core/load_module.h"
#include    "switch.h"
#include    "topology-types.h"
#include    "vec3.h"

#include    <topology-trajectory-device.h>

#include    <cstddef>
#include    <cstdio>
#include    <filesystem.h>

#include    <algorithm>
#include    <functional>
#include    <mutex>

#include    <Journal.h>
#include    <fstream>
#include    <sstream>
#include    <physics.h>

static bool get_non_empty_lines_from_file(
    const QString& path,
    std::vector<std::string>& lines
)
{
    std::ifstream file(path.toStdString());
    if (!file.is_open())
    {
        Journal::instance()->error("File " + path + " not found");
        return false;
    }

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);

        if (!line.empty())
        {
            lines.emplace_back(std::move(line));
        }
    }

    return true;
}

//------------------------------------------------------------------------------
/// Общий кэш конфигурации профилей пути маршрута: читается один раз при
/// загрузке первой траектории (loadRailProfile), далее используется
/// всеми траекториями, в т.ч. при генерации неровностей стрелок
//------------------------------------------------------------------------------
namespace
{
track::TrackProfileConfig shared_profile_config;
std::once_flag shared_profile_once;
bool shared_profile_loaded = false;
} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory::Trajectory(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory::~Trajectory() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::load(const QString &route_dir, const QString &traj_name,
                      std::vector<module_cfg_t>& modules, bool solve_errors)
{
    const QString path =
        QDir::toNativeSeparators(route_dir) +
        QDir::separator() + "topology" +
        QDir::separator() + "trajectories" +
        QDir::separator() + traj_name + ".traj";

    std::vector<std::string> lines;
    if (!get_non_empty_lines_from_file(path, lines))
    {
        return false;
    }

    if (lines.size() < 2)
    {
        Journal::instance()->error(QString("TOPOLOGY WARNING: No tracks in trajectory %1").arg(traj_name));
        if (solve_errors)
        {
            return false;
        }
    }

    dvec3 p0;
    double railway_coord0;

    // Линия, описывающая начальную точку трека
    std::istringstream ss_begin(lines[0]);
    ss_begin >> p0.x >> p0.y >> p0.z >> railway_coord0;

    const std::size_t lines_size = lines.size();
    for (std::size_t i = 1; i < lines_size; ++i)
    {
        dvec3 p1;
        double railway_coord1;

        // Следующая линия описывает конечную точку трека
        std::istringstream ss_end(lines[i]);
        ss_end >> p1.x >> p1.y >> p1.z >> railway_coord1;

        // Проверка совпадения точек p0 и p1
        const dvec3 dp = p1 - p0;

        // Откидываем сантиметровые треки и меньше
        if (solve_errors && (length(dp) <= 0.01))
        {
            const QString msg = QString("TOPOLOGY WARNING: "
                "Points %1 and %2 match in trajectory %3")
                .arg(i - 1, 4)
                .arg(i, 4)
                .arg(traj_name);

            Journal::instance()->error(msg);

            p0 = p1;
            railway_coord0 = railway_coord1;

            continue;
        }

        // Конструируем трек
        track_t& track = tracks.emplace_back(track_t(p0, p1));

        // Железнодорожный пикетаж
        track.railway_coord0 = railway_coord0;
        track.railway_coord1 = railway_coord1;

        // Обновляем траекторную координату начала трека
        track.traj_coord = len;

        // Обновляем длину траектории
        len += track.len;

        p0 = p1;
        railway_coord0 = railway_coord1;
    }

    // Заполняем имя траектории (по имени файла, где она хранится)
    name = traj_name;

    // Профиль вертикальных неровностей пути (ТЗ "Неровности пути")
    loadRailProfile(route_dir);

    // Загрузка модулей к траектории
    if (modules.empty())
    {
        Journal::instance()->warning("No modules for trajectory " + traj_name);
        return true;
    }

    const FileSystem& fs = FileSystem::getInstance();

    static QHash<QString, GetModuleFuncPtr> get_module_funcs;

    for (module_cfg_t& mc : modules)
    {
        // Загружаем dll модуль путевой инфраструктуры
        QString module_path = QString(fs.getModulesDir().c_str()) +
                                      QDir::separator() +
                                      mc.module_name;

        GetModuleFuncPtr get_module_func;

        const auto found_it = get_module_funcs.find(module_path);
        if (found_it == get_module_funcs.end())
        {
            get_module_func = load_get_module_func(module_path);
            get_module_funcs[module_path] = get_module_func;
        }
        else
        {
            get_module_func = found_it.value();
        }

        if (!get_module_func)
        {
            Journal::instance()->error("Failed to load module " + mc.module_name + " for trajectory " + traj_name);
            continue;
        }

        TrajectoryDevice* module = (TrajectoryDevice*)get_module_func();
        if (!module)
        {
            Journal::instance()->error("Failed to load module " + mc.module_name + " for trajectory " + traj_name);
            continue;
        }

        Journal::instance()->info("Loaded module " + mc.module_name + " for trajectory " + traj_name);

        // Указываем модулю, что он относится к этой траектории
        module->setTrajectory(this);

        // Конфигурируем модуль
        module->load_config(mc.cfg);

        // Добавляем модуль в список оборудования путевой инфраструктуры
        devices.push_back(module);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Trajectory::getName() const
{
    return name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Trajectory::getLength() const
{
    return len;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setFwdSwitch(Switch *switch_ptr)
{
    fwd_switch = switch_ptr;

    // Неровность крестовины у конца, подключённого к стрелке
    // (генерация после привязки: конфиг уже загружен loadRailProfile)
    if (switch_ptr != nullptr)
    {
        generateSwitchIrregularity(true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setBwdSwitch(Switch *switch_ptr)
{
    bwd_switch = switch_ptr;

    if (switch_ptr != nullptr)
    {
        generateSwitchIrregularity(false);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch* Trajectory::getNextSwitch(dir_t& dir) const
{
    if (dir == FWD)
    {
        if (fwd_switch)
        {
            dir = static_cast<dir_t>(dir * fwd_switch->getTrajOrientation(this));
            return fwd_switch;
        }
    }
    else if (dir == BWD)
    {
        if (bwd_switch)
        {
            dir = static_cast<dir_t>(dir * bwd_switch->getTrajOrientation(this));
            return bwd_switch;
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setInRoute(bool is_route)
{
    if (is_route)
    {
        in_route = true;
    }
    else
    {
        in_route = false;
        in_route_by_signal_fwd = nullptr;
        in_route_by_signal_bwd = nullptr;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::isInRoute() const
{
    return in_route;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setBusy(size_t idx, double coord_begin, double coord_end)
{
    if ((coord_begin < len) && (coord_end > 0.0) && (coord_begin < coord_end))
    {
        vehicles_coords.insert(idx, {coord_begin, coord_end});
    }
    else
    {
        vehicles_coords.remove(idx);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::clearBusy()
{
    vehicles_coords.clear();

    for (TrajectoryDevice* traj_device : devices)
    {
        traj_device->clearLinks();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setBusyState(bool busy_state)
{
    is_busy = busy_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::isBusy() const
{
    return is_busy;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::isBusy(double coord_begin, double coord_end) const
{
    for (const auto& vehicle_coord : vehicles_coords)
    {
        if ((vehicle_coord[1] >= coord_begin) &&
            (vehicle_coord[0] <= coord_end))
        {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Trajectory::getBusyVehicle(double &distance, double coord, double search_distance, dir_t direction) const
{
    double coord_begin = coord;
    double coord_end = coord;

    if (direction == BWD)
    {
        coord_begin = coord_begin - search_distance;
        if (is_busy)
        {
            double min_distance = search_distance;
            int idx = -1;
            for (auto vc_it = vehicles_coords.begin(); vc_it != vehicles_coords.end(); ++vc_it)
            {
                if ((vc_it.value()[1] >= coord_begin) && (vc_it.value()[1] <= coord_end))
                {
                    double d = coord_end - vc_it.value()[1];
                    if (min_distance > d)
                    {
                        min_distance = d;
                        idx = vc_it.key();
                    }
                }
            }
            if (idx >= 0)
            {
                distance = distance + min_distance;
                return idx;
            }
        }

        // Проверяем переход на предыдущую траекторию
        if (coord_begin < 0.0)
        {
            distance = distance + coord_end;

            const Switch* bwd_sw = getNextSwitch(direction);
            if (bwd_sw == nullptr)
                return -1;

            const Trajectory *traj = bwd_sw->getNextTraj(direction);
            if (traj == nullptr)
                return -1;

            coord = (direction == FWD) ? 0.0 : traj->getLength();
            return traj->getBusyVehicle(distance, coord, -coord_begin, direction);
        }
    }

    if (direction == FWD)
    {
        coord_end = coord_end + search_distance;
        if (is_busy)
        {
            double min_distance = search_distance;
            int idx = -1;
            for (auto vc_it = vehicles_coords.begin(); vc_it != vehicles_coords.end(); ++vc_it)
            {
                if ((vc_it.value()[0] >= coord_begin) && (vc_it.value()[0] <= coord_end))
                {
                    double d = vc_it.value()[0] - coord_begin;
                    if (min_distance > d)
                    {
                        min_distance = d;
                        idx = vc_it.key();
                    }
                }
            }
            if (idx >= 0)
            {
                distance = distance + min_distance;
                return idx;
            }
        }

        // Проверяем переход на следующую траекторию
        if (coord_end > len)
        {
            distance = distance + len - coord_begin;

            const Switch* fwd_sw = getNextSwitch(direction);
            if (fwd_sw == nullptr)
                return -1;

            const Trajectory *traj = fwd_sw->getNextTraj(direction);
            if (traj == nullptr)
                return -1;

            coord = (direction == FWD) ? 0.0 : traj->getLength();
            return traj->getBusyVehicle(distance, coord, coord_end - len, direction);
        }
    }

    distance = distance + search_distance;
    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::getBusyCoords(double &busy_begin_coord, double &busy_end_coord) const
{
    busy_begin_coord = len;
    busy_end_coord = 0.0;

    if (is_busy)
    {
        for (const auto& vehicle_coord : vehicles_coords)
        {
            if (busy_begin_coord > vehicle_coord[0])
                busy_begin_coord = vehicle_coord[0];

            if (busy_end_coord < vehicle_coord[1])
                busy_end_coord = vehicle_coord[1];
        }
    }
}

//------------------------------------------------------------------------------
// Вернуть все треки траектории
//------------------------------------------------------------------------------
const std::vector<track_t>& Trajectory::getTracks() const
{
    return tracks;
}

//------------------------------------------------------------------------------
// Вернуть первый трек траектории
//------------------------------------------------------------------------------
const track_t& Trajectory::getFirstTrack() const
{
    return tracks.front();
}

//------------------------------------------------------------------------------
// Вернуть последний трек траектории
//------------------------------------------------------------------------------
const track_t& Trajectory::getLastTrack() const
{
    return tracks.back();
}

//------------------------------------------------------------------------------
// Получить оборудование путевой инфраструктуры на этой траектории
//------------------------------------------------------------------------------
const std::vector<TrajectoryDevice*>& Trajectory::getTrajectoryDevices() const
{
    return devices;
}

//------------------------------------------------------------------------------
// Светофор вперёд, включающий данную траекторию в маршрут ДЦ
//------------------------------------------------------------------------------
Signal* Trajectory::getRouteBySignalFwd() const
{
    return in_route_by_signal_fwd;
}

//------------------------------------------------------------------------------
// Светофор вперёд, включающий данную траекторию в маршрут ДЦ
//------------------------------------------------------------------------------
void Trajectory::setRouteBySignalFwd(Signal* signal)
{
    in_route_by_signal_fwd = signal;
}

//------------------------------------------------------------------------------
// Светофор назад, включающий данную траекторию в маршрут ДЦ
//------------------------------------------------------------------------------
Signal* Trajectory::getRouteBySignalBwd() const
{
    return in_route_by_signal_bwd;
}

//------------------------------------------------------------------------------
// Светофор назад, включающий данную траекторию в маршрут ДЦ
//------------------------------------------------------------------------------
void Trajectory::setRouteBySignalBwd(Signal* signal)
{
    in_route_by_signal_bwd = signal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::step(double t, double dt)
{
    // Необходимость рассылки состояния траектории
    bool send = false;

    // Обновляем занятость подвижным составом
    if (is_busy == vehicles_coords.empty())
    {
        // Запомним предыдущее состояние занятости
        prev_is_busy = is_busy;
        // Обновим его
        is_busy = !vehicles_coords.empty();
        send = true;

        // Занятая траектория исключается из маршрута ДЦ
        if (is_busy)
        {
            setInRoute(false);
            // Если пока еще занято, то определяем первого из списка,
            // при освобождени траектории он же станет и последним
            last_bused_index = vehicles_coords.firstKey();
        }
    }

    // Обновляем занятость диспетчерскими маршрутами
    if (prev_in_route != in_route)
    {
        prev_in_route = in_route;
        send = true;
    }

    // Симуляция модулей путевой инфраструктуры
    for (auto traj_device : devices)
    {
        traj_device->step(t, dt);
    }

    // Рассылка нового состояния траектории
    if (send)
    {
        traj_busy_state_t new_state;
        new_state.name = name;
        new_state.is_busy = is_busy;
        new_state.in_route = in_route;
        emit sendTrajBusyState(new_state.serialize());
    }

    // Если изменилась занятость
    if (is_busy != prev_is_busy)
    {
        // Если свободность сменена на занятость
        if (is_busy)
        {
            // Значит какая-то ПЕ только что на нее заехала и мы
            // определяем индекс мерзавки
            int v_idx = vehicles_coords.firstKey();

            // и шлем его топологии, чтобы разобралась из какого она поезда
            emit sigTrajChangeState(v_idx, is_busy, name);
        }
        else
        {
            // посылаем топологии индекc последней ПЕ, занимавшей данную траекторию
            emit sigTrajChangeState(last_bused_index, is_busy, name);
        }

        prev_is_busy = is_busy;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Trajectory::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Кладем в буфер имя, длину и признак занятости
    stream << name << len << is_busy << in_route;

    // кладем туда же число треков
    stream << static_cast<uint32_t>(tracks.size());

    // Последовательно сериализум треки
    for (const track_t& track : tracks)
    {
        stream << track.serialize();
    }

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    // Восстанавливаем имя длину и признак занятости
    stream >> name;
    stream >> len;
    stream >> is_busy;
    stream >> in_route;

    // Восстанавливаем число треков
    uint32_t tracks_count;
    stream >> tracks_count;

    // Восстанавливаем треки
    for (quint32 i = 0; i < tracks_count; ++i)
    {
        QByteArray track_data;
        stream >> track_data;

        track_t track;
        track.deserialize(track_data);

        tracks.push_back(track);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::findTrajectoryAtCoord(Trajectory*& cur_traj, double& coord, dir_t& orient)
{
    double coord_off;
    return findTrajectoryAtCoord(cur_traj, coord, coord_off, orient);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::findTrajectoryAtCoord(Trajectory*& cur_traj, double& coord, double &coord_off, dir_t& orient)
{
    dir_t move_dir;
    while (true)
    {
        if (coord < 0.0)
        {
            // Если траекторная координата меньше нуля - заехали за стрелку сзади
            move_dir = BWD;
            // Запоминаем вылет за пределы траектории
            coord_off = coord;
        }
        else
        {
            if (coord > cur_traj->getLength())
            {
                // Если траекторная координата превысила длину траектории - заехали за стрелку спереди
                move_dir = FWD;
                // Запоминаем вылет за пределы траектории
                coord_off = coord - cur_traj->getLength();
            }
            else
            {
                // УРА! Находимся в пределах траектории: выходим
                coord_off = 0.0;
                return true;
            }
        }

        // Отслеживаем разворот ориентации траектории
        dir_t new_dir = move_dir;

        // Получаем указатель на стрелку в конце траектории
        const Switch* next_sw = cur_traj->getNextSwitch(new_dir);
        if (next_sw == nullptr)
        {
            // Если коннектора нет, выходим
            coord = coord - coord_off;
            return false;
        }

        // Получаем указатель на ту траекторию, с которой нас соединяет стрелка
        Trajectory* next_traj = next_sw->getNextTraj(new_dir);

        // Если за стрелкой нет траектории,
        // остаёмся на исходной траектории, останавливаемся на краю и выходим
        if (next_traj == nullptr)
        {
            coord = coord - coord_off;
            return false;
        }

        // Обновляем текущую траекторию
        cur_traj = next_traj;
        if (new_dir != move_dir)
        {
            // Если ориентация траектории изменилась, разворачиваемся
            orient = static_cast<dir_t>(-orient);
            coord_off = -coord_off;
        }

        if (new_dir == BWD)
        {
            // Если смещаемся назад, начинаем отсчёт с конца траектории
            coord = coord_off + cur_traj->getLength();
        }
        else
        {
            coord = coord_off;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
profile_point_t Trajectory::getPosition(double traj_coord, int direction) const
{
    profile_point_t pp;

    track_t cur_track = track_t();
    track_t prev_track = track_t();
    track_t next_track = track_t();

    findTracks(traj_coord, cur_track, prev_track, next_track);

    double dir = static_cast<double>(direction);

    pp.position = cur_track.begin_point +
                  cur_track.orth * (traj_coord - cur_track.traj_coord);

    pp.inclination = cur_track.inclination * dir;

    // Относительное перемещение вдоль текущего трека от 0.0 до 1.0
    double rel_motion = (traj_coord - cur_track.traj_coord) / cur_track.len;
    // Железнодорожный пикетаж
    pp.railway_coord = cur_track.railway_coord0 +
                       rel_motion * (cur_track.railway_coord1 - cur_track.railway_coord0);

    // Поворачиваем ориентацию к соседнему треку
    if (cur_track.len < 30.0)
    {
        // На треках короче 30 метров поворачиваем неперерывно
        // Плавное изменение кривизны от угла с предыдущем треком к углу со следующим
        pp.curvature = (1.0 - rel_motion) * calc_curvature(prev_track, cur_track) +
                       rel_motion * calc_curvature(cur_track, next_track);
    }
    else
    {
        // Треки длиннее 30 метров считаем прямыми в середине
        // Поворачиваем на первых и последних 15 метрах
        double track_coord = traj_coord - cur_track.traj_coord;
        if (track_coord < 15.0)
        {
            // Поворачиваем на первых 15 метрах
            // rel_motion от 0.0 до 0.5
            rel_motion = track_coord / 30.0;
            // Плавное изменение кривизны от угла с предыдущем треком к нулю
            double curv = (1.0 - track_coord / 15.0) *
                          calc_curvature(prev_track, cur_track);
            pp.curvature = (curv > 1e-5) ? curv : 0.0;
        }
        else
        {
            if (track_coord > (cur_track.len - 15.0))
            {
                // Поворачиваем на последних 15 метрах
                // rel_motion от 0.5 до 1.0
                rel_motion = 1.0 - (cur_track.len - track_coord) / 30.0;
                // Плавное изменение кривизны от нуля к углу со следующим треком
                double curv = ((15.0 + track_coord - cur_track.len) / 15.0) *
                              calc_curvature(cur_track, next_track);
                pp.curvature = (curv > 1e-5) ? curv : 0.0;
            }
            else
            {
                // В середине длинного трека движемся вдоль него
                pp.curvature = 0.0;
                pp.orth = cur_track.orth * dir;
                pp.right = cur_track.trav * dir;
                pp.up = cur_track.up;
                return pp;
            }
        }
    }

    if (rel_motion < 0.5)
    {
        pp.orth = cur_track.orth * (0.5 + rel_motion) * dir;
        pp.orth += prev_track.orth * (0.5 - rel_motion) * dir;

        pp.right = cur_track.trav * (0.5 + rel_motion) * dir;
        pp.right += prev_track.trav * (0.5 - rel_motion) * dir;

        pp.up = cur_track.up * (0.5 + rel_motion);
        pp.up += prev_track.up * (0.5 - rel_motion);

        pp.orth = normalize(pp.orth);
        pp.right = normalize(pp.right);
        pp.up = normalize(pp.up);
        return pp;
    }
    else
    {
        pp.orth = cur_track.orth * (1.5 - rel_motion) * dir;
        pp.orth += next_track.orth * (rel_motion - 0.5) * dir;

        pp.right = cur_track.trav * (1.5 - rel_motion) * dir;
        pp.right += next_track.trav * (rel_motion - 0.5) * dir;

        pp.up = cur_track.up * (1.5 - rel_motion);
        pp.up += next_track.up * (rel_motion - 0.5);

        pp.orth = normalize(pp.orth);
        pp.right = normalize(pp.right);
        pp.up = normalize(pp.up);
        return pp;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::loadRailProfile(const QString& route_dir)
{
    // Конфиг профилей маршрута читается один раз на все траектории
    std::call_once(shared_profile_once, [&route_dir]()
    {
        QString error;
        if (!track::loadTrackProfileConfig(route_dir,
                                           shared_profile_config,
                                           &error))
        {
            Journal::instance()->warning(
                "Track profile config not loaded: " + error);
        }

        shared_profile_loaded = true;
    });

    //--- Возвышение наружного рельса (Б16): свойство ПУТИ ---
    // Трекам, НАЧИНАЮЩИМСЯ внутри зоны [Cant], назначается постоянное
    // возвышение зоны; getCant() линейно интерполирует между соседними
    // треками - граничные треки дают переходный отвод возвышения
    // (длина отвода = длина трека). Знак CantMm: "+" - левый рельс выше
    for (track_t& track : tracks)
    {
        track.cant_mm = 0.0;

        for (const track::CantZone& zone : shared_profile_config.cants)
        {
            const bool zone_here = zone.traj_name.isEmpty() ||
                    zone.traj_name == "*" || zone.traj_name == name;

            if (zone_here &&
                track.traj_coord >= zone.begin &&
                track.traj_coord < zone.end)
            {
                // Перекрывающиеся зоны суммируются (редкий случай)
                track.cant_mm += zone.cant_mm;
            }
        }
    }

    if (!shared_profile_config.enabled)
        return;

    // Состояние пути: персональное для траектории или общий уровень
    track::Condition condition = shared_profile_config.condition;

    for (const auto& traj_condition : shared_profile_config.traj_conditions)
    {
        if (traj_condition.first == name)
        {
            condition = traj_condition.second;
            break;
        }
    }

    // Явные неровности: общие для всех путей и привязанные к этой траектории
    std::vector<track::Irregularity> irregularities =
            shared_profile_config.explicit_irregularities;

    for (const auto& named : shared_profile_config.named_irregularities)
    {
        if (named.first == name)
            irregularities.push_back(named.second);
    }

    rail_profile.generate(name.toStdString(),
                          len,
                          irregularities,
                          shared_profile_config.joints,
                          shared_profile_config.noise,
                          condition,
                          shared_profile_config.seed);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Trajectory::getRailHeight(double traj_coord, int side) const
{
    const double coord = std::min(len, std::max(0.0, traj_coord));
    return rail_profile.railHeight(coord, side);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Trajectory::getLateralOffset(double traj_coord) const
{
    const double coord = std::min(len, std::max(0.0, traj_coord));
    return rail_profile.lateralOffset(coord);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Trajectory::getCant(double traj_coord) const
{
    if (tracks.empty())
        return 0.0;

    // Бинарный поиск трека, содержащего координату: треки отсортированы
    // по возрастанию traj_coord (координата начала трека)
    const auto upper = std::upper_bound(tracks.begin(),
                                        tracks.end(),
                                        traj_coord,
                                        [](double value, const track_t& track)
    {
        return value < track.traj_coord;
    });

    const auto idx = static_cast<std::size_t>(
                (upper == tracks.begin()) ? 0 : (upper - tracks.begin()) - 1);

    const track_t& cur = tracks[idx];
    const double next_cant = (idx + 1 < tracks.size())
            ? tracks[idx + 1].cant_mm
            : cur.cant_mm;

    // Линейная интерполяция возвышения по длине трека: значение на
    // начале трека -> значение на начале следующего. На граничном треке
    // это переходный отвод возвышения (плавный вход/выход кривой)
    const double rel = (cur.len > 1e-6)
            ? std::min(std::max((traj_coord - cur.traj_coord) / cur.len, 0.0), 1.0)
            : 0.0;

    return cur.cant_mm + (next_cant - cur.cant_mm) * rel;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::generateSwitchIrregularity(bool at_fwd_end)
{
    // Топология знает стрелки: у траектории, подключённой к стрелке,
    // зона перевода (остряки - крестовина - контррельсы) лежит у
    // соответствующего конца. Ставим неровность Switch (двойной импульс
    // + жёсткий удар крестовины) центром зоны у конца траектории
    if (!shared_profile_loaded ||
        !shared_profile_config.enabled ||
        !shared_profile_config.switch_irregularity)
    {
        return;
    }

    bool& placed = at_fwd_end ? switch_irreg_fwd : switch_irreg_bwd;

    if (placed)
        return;

    const double zone_len = std::max(shared_profile_config.switch_length, 1.0);

    if (len < 2.0 * zone_len)
        return;

    track::Irregularity sw;
    sw.type = track::IrregularityType::Switch;
    sw.side = track::RailSide::Both;
    sw.amplitude = shared_profile_config.switch_amplitude;
    sw.length = zone_len;
    sw.coord = at_fwd_end ? (len - 0.5 * zone_len) : (0.5 * zone_len);
    // Износ крестовины: детерминированный разброс по имени траектории
    // и стороне подключения (паттерн автогенерации стыков)
    const std::uint32_t bits = static_cast<std::uint32_t>(
                std::hash<std::string>{}(name.toStdString()) ^
                (at_fwd_end ? 0x9e3779b9u : 0x85ebca6bu));
    sw.wear = 0.3 + 0.7 * static_cast<double>(bits % 1000u) / 1000.0;

    rail_profile.addIrregularity(sw);
    placed = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::damageTrack(double factor)
{
    rail_profile.degradeTrack(factor);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::addTonnage(double traj_coord, double mass_tonnes,
                            double distance_m)
{
    rail_profile.addPassage(traj_coord, mass_tonnes, distance_m);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::findTracks(double traj_coord,
                            track_t& cur_track,
                            track_t& prev_track,
                            track_t& next_track) const
{
    if (tracks.empty())
    {
        return;
    }

    // Исходим из того, что случаи traj_coord < 0 и traj_coord > len не допускаются.
    // В этом случае, текущий трек это трек на данной траектории, и если
    // он последний, то следующий трек - это первый трек сделующей траектории

    // Обрабатываем случай, когда мы на первом треке
    if (traj_coord <= tracks.front().len)
    {
        cur_track = tracks.front();

        // Ищем предыдущий трек на предыдущей по топологии траектории
        prev_track = findNextTrack(cur_track, BWD);

        auto next = (tracks.begin() + 1);
        if (next == tracks.end())
        {
            // Обрабатываем случай единственного трека в траектории
            next_track = findNextTrack(cur_track, FWD);
        }
        else
        {
            // Следующий трек в данной траектории
            next_track = *next;
        }
        return;
    }

    // Обрабатываем случай, коогда мы оказываемся на последнем треке
    if (traj_coord >= tracks.back().traj_coord)
    {
        cur_track = tracks.back();

        // Ищем следующий трек на следующей по топологии траектории
        next_track = findNextTrack(cur_track, FWD);

        // По идее случай единственного трека обработан в условии выше,
        // и можно смело брать предпоследний трек
        prev_track = *(tracks.end() - 2);
        return;
    }

    // Если мы не на первом или последнем треке, ищем на каком мы треке
    // бинарным поиском
    const track_t* track = nullptr;

    size_t left_idx = 0;
    size_t right_idx = tracks.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = &tracks[idx];

        if (traj_coord <= track->traj_coord)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    cur_track = tracks[idx];
    prev_track = tracks[idx - 1];
    next_track = tracks[idx + 1];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
track_t Trajectory::findNextTrack(const track_t& cur_track, dir_t dir) const
{
    dir_t new_dir = dir;

    if (const Switch* next_sw = getNextSwitch(new_dir))
    {
        if (const Trajectory* next_traj = next_sw->getNextTraj(new_dir))
        {
            if (new_dir == dir)
            {
                if (new_dir == FWD)
                {
                    return next_traj->getFirstTrack();
                }
                if (new_dir == BWD)
                {
                    return next_traj->getLastTrack();
                }
            }
            else
            {
                if (new_dir == FWD)
                {
                    return createReversedTrack(next_traj->getFirstTrack());
                }
                if (new_dir == BWD)
                {
                    return createReversedTrack(next_traj->getLastTrack());
                }
            }
        }
    }

    return createFakeTrack(cur_track, dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
track_t Trajectory::createFakeTrack(const track_t& cur_track, dir_t dir) const
{
    track_t fake_track;

    if (dir == FWD)
    {
        fake_track.begin_point = cur_track.end_point;
        fake_track.end_point += cur_track.orth * cur_track.len;
    }
    else if (dir == BWD)
    {
        fake_track.begin_point -= cur_track.orth * cur_track.len;
        fake_track.end_point = cur_track.begin_point;
    }

    fake_track.orth = cur_track.orth;
    fake_track.trav = cur_track.trav;
    fake_track.up = cur_track.up;
    fake_track.len = cur_track.len;

    // Прочие параметры трека не важны, когда он используется как соседний
    return fake_track;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
track_t Trajectory::createReversedTrack(const track_t& track) const
{
    track_t fake_track;

    // Параметры наоборот
    fake_track.begin_point = track.end_point;
    fake_track.end_point = track.begin_point;
    fake_track.orth = -track.orth;
    fake_track.trav = -track.trav;

    // Параметры без изменений
    fake_track.up = track.up;
    fake_track.len = track.len;

    // Прочие параметры трека не важны, когда он используется как соседний
    return fake_track;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Trajectory::calc_curvature(const track_t& track0, const track_t& track1) const
{
    double curvature = 0.0;

    // Направление первого трека
    double A0 = track0.orth.x;
    double B0 = track0.orth.y;

    // Направление второго трека
    double A1 = track1.orth.x;
    double B1 = track1.orth.y;

    double det = A0 * B1 - A1 * B0;

    // Если треки параллельны - кривизна нулевая
    if ( qAbs(det) < 1e-5 )
    {
        //Journal::instance()->info(QString("det=%1 | curv=0.0").arg(det, 15, 'f', 12));
        return 0.0;
    }

    // Центр первого трека
    dvec3 S0 = -track0.orth * 0.5 * track0.len;
    double D0 = A0 * S0.x + B0 * S0.y;

    // Центр второго трека
    dvec3 S1 = track1.orth * 0.5 * track1.len;
    double D1 = A1 * S1.x + B1 * S1.y;

    double xC = (B0 * D1 - B1 * D0) / det;
    double yC = (A0 * D1 - A1 * D0) / det;

    double rho = std::sqrt(xC * xC + yC * yC);

    curvature = 1.0 / rho;

    //Journal::instance()->info(QString("det=%1 | curv=%2 | r=%3").arg(det, 15, 'f', 12).arg(curvature, 15, 'f', 12).arg(rho, 15, 'f', 1));
    return curvature;
}
