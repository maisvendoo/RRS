#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>

#include    "track.h"
#include    "connector.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Trajectory : public QObject
{
    Q_OBJECT

public:

    Trajectory(QObject *parent = Q_NULLPTR);

    ~Trajectory();

    bool load(QString route_dir, QString traj_name);

    QString getName() const { return name; }

    vec3d getPosition(double x, vec3d &attitude);

protected:

    QString                 name;

    double                  length;

    Connector               *fwdConnector;

    Connector               *bwdConnector;

    std::vector<track_t>    tracks;

    track_t findTrack(double x, track_t &next_track);
};

#endif // TRAJECTORY_H
