#include    "trajectory.h"
#include    "switch.h"
#include    "topology-types.h"

#include    <filesystem.h>

#include    <fstream>
#include    <Journal.h>
#include    <physics.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory::Trajectory(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory::~Trajectory()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trajectory::load(const QString &route_dir, const QString &traj_name,
                      std::vector<module_cfg_t> modules, bool solve_errors)
{
    QString path = QDir::toNativeSeparators(route_dir) +
                   QDir::separator() + "topology" +
                   QDir::separator() + "trajectories" +
                   QDir::separator() + traj_name + ".traj";

    std::ifstream stream(path.toStdString(), std::ios::in);

    if (!stream.is_open())
    {
        Journal::instance()->error("File " + path + " not found");
        return false;
    }

    std::vector<std::string> lines;

    // Читаем непустые линии из файла точек траектории
    while (!stream.eof())
    {
        std::string line = "";
        std::getline(stream, line);

        if (line.empty())
            continue;

        lines.push_back(line);
    }

    for (size_t i = 0; i < lines.size() - 1; ++i)
    {
        // Линия, описывающая начальную точку трека
        std::istringstream ss_begin(lines[i]);
        // следующая линия описывает конечную точку трека
        std::istringstream ss_end(lines[i+1]);

        // Читаем начальную и конечную точки
        dvec3 p0;
        double railway_coord0;

        ss_begin >> p0.x >> p0.y >> p0.z >> railway_coord0;

        dvec3 p1;
        double railway_coord1;

        ss_end >> p1.x >> p1.y >> p1.z >> railway_coord1;

        // Проверка совпадения точек p0 и p1
        dvec3 dp = p1 - p0;

        // Откидываем сантиметровые треки и меньше
        if (solve_errors && (length(dp) <= 0.01))
        {
            QString msg = QString("TOPOLOGY WARNING: Points %1 and %2 match in trajectory %3")
                              .arg(i, 4)
                              .arg(i+1, 4)
                              .arg(traj_name);

            Journal::instance()->error(msg);
            continue;
        }

        // Конструируем трек
        track_t track(p0, p1);

        // Железнодорожный пикетаж
        track.railway_coord0 = railway_coord0;
        track.railway_coord1 = railway_coord1;

        // Обновляем траекторную координату начала трека
        track.traj_coord = len;

        // Обновляем длину траектории
        len += track.len;

        tracks.push_back(track);
    }

    // Заполняем имя траектории (по имени файла, где она хранится)
    name = traj_name;

    // Загрузка модулей к траектории
    if (modules.empty())
    {
        Journal::instance()->warning("No modules for trajectory " + traj_name);
        return true;
    }

    FileSystem &fs = FileSystem::getInstance();
    for (auto mc = modules.begin(); mc != modules.end(); ++mc)
    {
        // Загружаем dll модуль путевой инфраструктуры
        QString module_path = QString(fs.getModulesDir().c_str()) +
                                      QDir::separator() +
                                      mc->module_name;
        TrajectoryDevice *module = loadTrajectoryDevice(module_path);

        if (module  == nullptr)
        {
            Journal::instance()->error(
                "Fail to load module " + mc->module_name + ".dll for trajectory " + traj_name);
            continue;
        }

        Journal::instance()->info(
            "Loaded module " + mc->module_name + ".dll for trajectory " + traj_name);

        // Указываем модулю, что он относится к этой траектории
        module->setTrajectory(this);

        // Конфигурируем модуль
        module->load_config(mc->cfg);

        // Добавляем модуль в список оборудования путевой инфраструктуры
        devices.push_back(module);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setFwdSwitch(Switch *switch_ptr)
{
    fwd_switch = switch_ptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::setBwdSwitch(Switch *switch_ptr)
{
    bwd_switch = switch_ptr;
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
    if (dir == BWD)
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
        vehicles_coords.insert(idx, {coord_begin, coord_end});
    else
        vehicles_coords.remove(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::clearBusy()
{
    vehicles_coords.clear();
    for (auto traj_device : devices)
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
    for (auto vehicle_coord : vehicles_coords)
    {
        if ((vehicle_coord[1] >= coord_begin) && (vehicle_coord[0] <= coord_end))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Trajectory::getBusyVehicle(double &distance, double coord, double search_distance, dir_t direction)
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

            Switch* bwd_sw = getNextSwitch(direction);
            if (bwd_sw == nullptr)
                return -1;

            Trajectory *traj = bwd_sw->getNextTraj(direction);
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

            Switch* fwd_sw = getNextSwitch(direction);
            if (fwd_sw == nullptr)
                return -1;

            Trajectory *traj = fwd_sw->getNextTraj(direction);
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
void Trajectory::getBusyCoords(double &busy_begin_coord, double &busy_end_coord)
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
QByteArray Trajectory::serialize()
{
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    QDataStream stream(&data);

    // Кладем в буфер имя, длину и признак занятости
    stream << name << len << is_busy << in_route;

    // кладем туда же число треков
    stream << static_cast<uint32_t>(tracks.size());

    // Последовательно сериализум треки
    for (auto track = tracks.begin(); track != tracks.end(); ++track)
    {
        stream << track->serialize();
    }

    return data.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

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
        Switch* next_sw = cur_traj->getNextSwitch(new_dir);
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
profile_point_t Trajectory::getPosition(double traj_coord, int direction)
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
void Trajectory::findTracks(double traj_coord,
                            track_t& cur_track,
                            track_t& prev_track,
                            track_t& next_track)
{
    if (tracks.size() == 0)
        return;

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
    track_t track;

    size_t left_idx = 0;
    size_t right_idx = tracks.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = tracks[idx];

        if (traj_coord <= track.traj_coord)
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
track_t Trajectory::findNextTrack(const track_t& cur_track, dir_t dir)
{
    dir_t new_dir = dir;

    if (Switch* next_sw = getNextSwitch(new_dir))
    {
        if (Trajectory* next_traj = next_sw->getNextTraj(new_dir))
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
track_t Trajectory::createFakeTrack(const track_t& cur_track, dir_t dir)
{
    track_t fake_track;

    if (dir == FWD)
    {
        fake_track.begin_point = cur_track.end_point;
        fake_track.end_point += cur_track.orth * cur_track.len;
    }
    if (dir == BWD)
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
track_t Trajectory::createReversedTrack(const track_t& track)
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
double Trajectory::calc_curvature(track_t& track0, track_t& track1)
{
    double curvature = 0.0;

    // Направление первого трека
    double A0 = track0.orth.x;
    double B0 = track0.orth.y;

    // Направление второго трека
    double A1 = track1.orth.x;
    double B1 = track1.orth.y;

    double det = A0*B1 - A1*B0;

    // Если треки параллельны - кривизна нулевая
    if ( qAbs(det) < 1e-5 )
    {
        //Journal::instance()->info(QString("det=%1 | curv=0.0").arg(det, 15, 'f', 12));
        return 0.0;
    }

    // Центр первого трека
    dvec3 S0 = - track0.orth * 0.5 * track0.len;
    double D0 = A0 * S0.x + B0 * S0.y;

    // Центр второго трека
    dvec3 S1 = track1.orth * 0.5 * track1.len;
    double D1 = A1 * S1.x + B1 * S1.y;

    double xC = (B0*D1 - B1*D0) / det;
    double yC = (A0*D1 - A1*D0) / det;

    double rho = std::sqrt(xC * xC + yC * yC);

    curvature = 1 / rho;

    //Journal::instance()->info(QString("det=%1 | curv=%2 | r=%3").arg(det, 15, 'f', 12).arg(curvature, 15, 'f', 12).arg(rho, 15, 'f', 1));
    return curvature;
}
