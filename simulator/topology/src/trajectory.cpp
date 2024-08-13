#include    <trajectory.h>
#include    <connector.h>

#include    <QDir>

#include    <fstream>

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
profile_point_t Trajectory::getPosition(double traj_coord, int direction)
{
    profile_point_t pp;

    track_t cur_track = track_t();
    track_t prev_track = track_t();
    track_t next_track = track_t();

    findTracks(traj_coord, cur_track, prev_track, next_track);

    pp.position = cur_track.begin_point +
                  cur_track.orth * (traj_coord - cur_track.traj_coord);

    double dir = static_cast<double>(direction);

    pp.inclination = cur_track.inclination * dir;

    // Относительное перемещение вдоль текущего трека
    double rel_motion = traj_coord / len;

    if (rel_motion < 0.5)
    {
        pp.orth = cur_track.orth * (0.5 + rel_motion) * dir;
        pp.orth += prev_track.orth * (0.5 - rel_motion) * dir;

        pp.right = cur_track.trav * (0.5 + rel_motion) * dir;
        pp.right += prev_track.trav * (0.5 - rel_motion) * dir;

        pp.up = cur_track.up * (0.5 + rel_motion);
        pp.up += prev_track.up * (0.5 - rel_motion);

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

        return pp;
    }

    return pp;
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

    // Обрабатываем случай, когда мы на первом треке
    if (traj_coord < (*tracks.begin()).len)
    {
        cur_track = *(tracks.begin());
        next_track = *(tracks.begin() + 1);

        // Если нет коннектора сзади
        if (bwd_connector == Q_NULLPTR)
        {
            prev_track = cur_track;
            prev_track.begin_point -= cur_track.orth * cur_track.len;
            prev_track.end_point -= cur_track.orth * cur_track.len;
            prev_track.traj_coord -= cur_track.len;
            return;
        }

        // Смотрим, какая траектория сзади
        Trajectory *prev_traj = bwd_connector->getBwdTraj();

        // Если сзади нет траектории
        if (prev_traj == Q_NULLPTR)
        {
            prev_track = cur_track;
            prev_track.begin_point -= cur_track.orth * cur_track.len;
            prev_track.end_point -= cur_track.orth * cur_track.len;
            prev_track.traj_coord -= cur_track.len;
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
            next_track = cur_track;
            next_track.begin_point += cur_track.orth * cur_track.len;
            next_track.end_point += cur_track.orth * cur_track.len;
            next_track.traj_coord += cur_track.len;
            return;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == Q_NULLPTR)
        {
            // Следующий трек сонаправлен текущему
            next_track = cur_track;
            next_track.begin_point += cur_track.orth * cur_track.len;
            next_track.end_point += cur_track.orth * cur_track.len;
            next_track.traj_coord += cur_track.len;
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
