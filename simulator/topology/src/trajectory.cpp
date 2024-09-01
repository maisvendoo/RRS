#include    <trajectory.h>
#include    <connector.h>

#include    <QDir>

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
bool Trajectory::load(const QString &route_dir, const QString &traj_name)
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

        ss_begin >> p0.x >> p0.y >> p0.z;

        dvec3 p1;

        ss_end >> p1.x >> p1.y >> p1.z;

        // Конструируем трек
        track_t track(p0, p1);

        // Обновляем траекторную координату начала трека
        track.traj_coord = len;

        // Обновляем длину траектории
        len += track.len;

        tracks.push_back(track);
    }

    // Заполняем имя траектории (по имени файла, где она хранится)
    name = traj_name;

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
    if ((coord_begin >= 0.0) && (coord_end <= len))
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
profile_point_t Trajectory::getPosition(double traj_coord, int direction)
{
    profile_point_t pp;

    track_t cur_track = track_t();
    track_t prev_track = track_t();
    track_t next_track = track_t();

    findTracks(traj_coord, cur_track, prev_track, next_track);

    double dir = static_cast<double>(direction);

    pp.curvature = calc_curvature(cur_track, next_track);

    pp.position = cur_track.begin_point +
                  cur_track.orth * (traj_coord - cur_track.traj_coord);


    pp.inclination = cur_track.inclination * dir;

    // Относительное перемещение вдоль текущего трека
    double rel_motion = (traj_coord - cur_track.traj_coord) / cur_track.len;

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

    if (rel_motion >= 0.5)
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

    return pp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trajectory::step(double t, double dt)
{
    if (is_busy == vehicles_coords.empty())
    {
        is_busy = !vehicles_coords.empty();
        //Journal::instance()->info(QString("Busy update: %1 %2 -> %3").arg(name).arg(!is_busy).arg(is_busy));

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
    stream << name << len << is_busy;

    // кладем туда же число треков
    stream << tracks.size();

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

    // Восстанавливаем число треков
    qsizetype tracks_count;
    stream >> tracks_count;

    // Восстанавливаем треки
    for (qsizetype i = 0; i < tracks_count; ++i)
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
        if (bwd_connector == Q_NULLPTR)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        // Смотрим, какая траектория сзади
        Trajectory *prev_traj = bwd_connector->getBwdTraj();

        // Если сзади нет траектории
        if (prev_traj == Q_NULLPTR)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        prev_track = prev_traj->getLastTrack();

        // Если нет соннектора впереди
        if (fwd_connector == Q_NULLPTR)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == Q_NULLPTR)
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
        if (bwd_connector == Q_NULLPTR)
        {
            prev_track = addFakeTrack(cur_track, false);
            return;
        }

        // Смотрим, какая траектория сзади
        Trajectory *prev_traj = bwd_connector->getBwdTraj();

        // Если сзади нет траектории
        if (prev_traj == Q_NULLPTR)
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
        if (fwd_connector == Q_NULLPTR)
        {
            // Следующий трек сонаправлен текущему
            next_track = addFakeTrack(cur_track, true);
            return;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == Q_NULLPTR)
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
double Trajectory::calc_curvature(track_t &cur_track, track_t &next_track)
{
    double curvature = 0.0;

    double A0 = cur_track.orth.x;
    double B0 = cur_track.orth.y;

    // Центр текущего трека
    dvec3 S0 = (cur_track.begin_point + cur_track.end_point) * 0.5;

    double D0 = A0 * S0.x + B0 * S0.y;

    double A1 = next_track.orth.x;
    double B1 = next_track.orth.y;

    // Центр следующего трека
    dvec3 S1 = (next_track.begin_point + next_track.end_point) * 0.5;

    double D1 = A1 * S1.x + B1 * S1.y;

    double det = A0*B1 - A1*B0;

    if ( qAbs(det) <= Physics::ZERO )
    {
        return 0.0;
    }

    double xC = (B0*D1 - B1*D0) / det;
    double yC = (A0*D1 - A1*D0) / det;

    double rho = std::sqrt( std::pow(S0.x - xC, 2.0) + std::pow(S0.y - yC, 2.0) );

    curvature = 1 / rho;

    return curvature;
}
