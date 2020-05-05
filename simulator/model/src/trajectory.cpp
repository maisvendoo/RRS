#include    "trajectory.h"

#include    <QFile>
#include    <QDir>
#include    <QTextStream>

#include    "physics.h"
#include    "connector.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory::Trajectory(QObject *parent) : QObject(parent)
  , name("")
  , length(0.0)
  , fwdConnector(Q_NULLPTR)
  , bwdConnector(Q_NULLPTR)
  , is_busy(false)
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
bool Trajectory::load(QString route_dir, QString traj_name)
{
    QString file_path = route_dir + QDir::separator() + "trajectories" +
            QDir::separator() + traj_name + ".trk";

    name = traj_name;

    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    while (!file.atEnd())
    {
        QString line = file.readLine();

        QTextStream ss(&line);

        vec3d p0;
        vec3d p1;

        ss >> p0.x() >> p0.y() >> p0.z()
           >> p1.x() >> p1.y() >> p1.z();

        track_t track = track_t(p0, p1);

        if (tracks.size() == 0)
        {
            track.x0 = 0.0;
            track.x1 = track.length;
        }
        else
        {
            size_t idx = tracks.size() - 1;
            track.x0 = tracks[idx].x1;
            track.x1 = track.x0 + track.length;
        }

        tracks.push_back(track);

        length += track.length;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double arg(double sin_x, double cos_x)
{
    return acos(cos_x) * sign(sin_x);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vec3d Trajectory::getPosition(double x, vec3d &attitude)
{
    vec3d pos;

    track_t track;
    track_t next_track;

    track = findTrack(x, next_track);

    pos = track.p0 + (x - track.x0) * track.orth;

    attitude.x() = asin(track.orth.z()) / Physics::RAD;
    attitude.y() = 0.0;
    attitude.z() = arg(track.trav.x(), track.trav.y()) / Physics::RAD;

    return pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
track_t Trajectory::findTrack(double x, track_t &next_track)
{
    if (tracks.size() == 0)
        return track_t();

    if (x < (*tracks.begin()).x0)
        return track_t();

    if (x >= (*(tracks.end() - 1)).x1 )
        return track_t();

    track_t track;

    size_t left_idx = 0;
    size_t right_idx = tracks.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        track = tracks[idx];

        if (x <= track.x0)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    track = tracks[idx];

    if (idx == tracks.size() - 1)
        next_track = track;
    else
        next_track = tracks[idx+1];

    return track;
}
