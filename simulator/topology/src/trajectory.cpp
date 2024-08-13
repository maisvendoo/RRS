#include    <trajectory.h>

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

        // Обновляем длину траектории
        len += track.len;

        tracks.push_back(track);
    }

    // Заполняем имя траектории (по имени файла, где она хранится)
    name = traj_name;

    return true;
}
