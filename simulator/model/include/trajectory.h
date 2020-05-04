#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>

#include    "track.h"

class       Connector;

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

    void setFwdConnector(Connector *conn) { this->fwdConnector = conn; }

    void setBwdConnector(Connector *conn) { this->bwdConnector = conn; }

    double getLength() const { return length; }

    Connector *getFwdConnector() const { return fwdConnector; }

    Connector *getBwdConnector() const { return bwdConnector; }

    void setBusy(double is_busy) { this->is_busy = is_busy; }

    bool isBusy() const { return is_busy; }

protected:

    QString         name;

    double          length;

    Connector       *fwdConnector;

    Connector       *bwdConnector;

    bool            is_busy;

    std::vector<track_t>    tracks;

    track_t findTrack(double x, track_t &next_track);
};

#endif // TRAJECTORY_H
