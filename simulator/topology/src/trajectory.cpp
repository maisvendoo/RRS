#include    <trajectory.h>
#include    <connector.h>

#include    <filesystem.h>

#include    <fstream>
#include    <Journal.h>
#include    <physics.h>
#include    <switch.h>

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
std::vector<TrajectoryDevice *> Trajectory::getTrajectoryDevices()
{
    return devices;
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
    if (is_busy)
    {
        for (auto vehicle_coord : vehicles_coords)
        {
            if ((vehicle_coord[1] >= coord_begin) && (vehicle_coord[0] <= coord_end))
                return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Trajectory::getBusyVehicle(double &distance, double coord, double search_distance, int direction)
{
    double coord_begin = coord;
    double coord_end = coord;
    if (direction == -1)
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

            if (bwd_connector == nullptr)
                return -1;

            Trajectory *traj = bwd_connector->getBwdTraj();
            if (traj == nullptr)
                return -1;

            return traj->getBusyVehicle(distance, traj->getLength(), -coord_begin, -1);
        }
    }
    else
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

            if (fwd_connector == nullptr)
                return -1;

            Trajectory *traj = fwd_connector->getFwdTraj();
            if (traj == nullptr)
                return -1;

            return traj->getBusyVehicle(distance, 0.0, coord_end - len, 1);
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
void Trajectory::step(double t, double dt)
{
    if (is_busy == vehicles_coords.empty())
    {
        is_busy = !vehicles_coords.empty();

        // Занятая траектория исключается из маршрута ДЦ
        if (is_busy)
        {
            in_route = false;
        }

        traj_busy_state_t new_state;
        new_state.name = name;
        new_state.is_busy = is_busy;
        emit sendTrajBusyState(new_state.serialize());
    }

    for (auto traj_device : devices)
    {
        traj_device->step(t, dt);
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
track_t addFakeTrack(track_t &cur_track, bool plus = true)
{
    track_t fake_track = cur_track;

    if (plus)
    {
        fake_track.begin_point += cur_track.orth * cur_track.len;
        fake_track.end_point += cur_track.orth * cur_track.len;
        fake_track.traj_coord += cur_track.len;
    }
    else
    {
        fake_track.begin_point -= cur_track.orth * cur_track.len;
        fake_track.end_point -= cur_track.orth * cur_track.len;
        fake_track.traj_coord -= cur_track.len;
    }

    return fake_track;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::findTracks(double traj_coord,
                            track_t &cur_track,
                            track_t &prev_track,
                            track_t &next_track)
{
    if (tracks.size() == 0)
        return;

    // Исходим из того, что случай traj_coord < 0 у нас недопускается.
    // В этом случае, текущий трек это трек на данной траектории, и если
    // он последний, то следующий трек - это первый трек сделующей траектории

    // Если у нас единственный трек, он де первый и он же последний
    if (tracks.size() == 1)
    {
        cur_track = *(tracks.begin());

        // Если нет коннектора сзади
        if (bwd_connector == nullptr)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        // Смотрим, какая траектория сзади
        Trajectory *prev_traj = bwd_connector->getBwdTraj();

        // Если сзади нет траектории
        if (prev_traj == nullptr)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        prev_track = prev_traj->getLastTrack();

        // Если нет соннектора впереди
        if (fwd_connector == nullptr)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == nullptr)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        next_track = next_traj->getFirstTrack();
        return;
    }


    // Обрабатываем случай, когда мы на первом треке
    if (traj_coord < (*tracks.begin()).len)
    {
        cur_track = *(tracks.begin());
        next_track = *(tracks.begin() + 1);

        // Если нет коннектора сзади
        if (bwd_connector == nullptr)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        // Смотрим, какая траектория сзади
        Trajectory *prev_traj = bwd_connector->getBwdTraj();

        // Если сзади нет траектории
        if (prev_traj == nullptr)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        prev_track = prev_traj->getLastTrack();
        return;
    }

    // Обрабатываем случай, коогда мы оказываемся на последнем треке
    if (traj_coord > (*(tracks.end() - 1)).traj_coord)
    {
        prev_track = *(tracks.end() - 2);
        cur_track = this->getLastTrack();

        // Если нет соннектора впереди
        if (fwd_connector == nullptr)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == nullptr)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        next_track = next_traj->getFirstTrack();
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
double Trajectory::calc_curvature(track_t &track0, track_t &track1)
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
