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

    while (!stream.eof())
    {
        std::string line = "";
        std::getline(stream, line);
        lines.push_back(line);
    }

    for (size_t i = 0; i < lines.size() - 1; ++i)
    {
        std::istringstream ss_begin(lines[i]);
        std::istringstream ss_end(lines[i+1]);

        dvec3 p0;

        ss_begin >> p0.x >> p0.y >> p0.z;

        dvec3 p1;

        ss_end >> p1.x >> p1.y >> p1.z;

        track_t track(p0, p1);

        tracks.push_back(track);
    }

    return true;
}
