#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>

#include    <track.h>

class Connector;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Trajectory : public QObject
{
    Q_OBJECT

public:

    Trajectory(QObject *parent = Q_NULLPTR);

    ~Trajectory();

    bool load(const QString &route_dir, const QString &traj_name);

private:

    QString name = "";

    double len = 0.0;

    bool is_busy = false;

    Connector *fwd_connector = Q_NULLPTR;

    Connector *bwd_connector = Q_NULLPTR;

    std::vector<track_t>    tracks;
};

#endif
