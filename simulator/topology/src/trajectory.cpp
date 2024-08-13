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
track_t Trajectory::findTracks(double traj_coord, track_t &next_track)
{
    if (tracks.size() == 0)
        return track_t();

    // Исходим из того, что случай traj_coord < 0 у нас недопускается.
    // В этом случае, текущий трек это трек на данной траектории, и если
    // он последний, то следующий трек - это первый трек сделующей траектории

    // Обрабатываем случай, коогда мы оказываемся на последнем треке
    if (traj_coord > (*(tracks.end() - 1)).traj_coord)
    {
        track_t track = this->getLastTrack();

        // Если нет соннектора впереди
        if (fwd_connector == Q_NULLPTR)
        {
            // Следующий трек сонаправлен текущему
            next_track = track;
            next_track.begin_point += track.orth * track.len;
            next_track.end_point += track.orth * track.len;
            next_track.traj_coord += track.len;
            return track;
        }

        // Смотрим, какая траектория впереди
        Trajectory *next_traj = fwd_connector->getFwdTraj();

        // Если впереди нет траектории
        if (next_traj == Q_NULLPTR)
        {
            // Следующий трек сонаправлен текущему
            next_track = track;
            next_track.begin_point += track.orth * track.len;
            next_track.end_point += track.orth * track.len;
            next_track.traj_coord += track.len;
            return track;
        }

        next_track = next_traj->getFirstTrack();
        return track;
    }

    // Если мы не на последнем треке, ищем на каком мы треке
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

    track = tracks[idx];
    next_track = tracks[idx + 1];

    return track_t();
}
