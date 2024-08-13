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

    QString getName() const
    {
        return name;
    }

    double getLength() const
    {
        return len;
    }

    void setFwdConnector(Connector *fwd_connector)
    {
        this->fwd_connector = fwd_connector;
    }

    void setBwdConnector(Connector *bwd_connector)
    {
        this->bwd_connector = bwd_connector;
    }

    Connector *getFwdConnector() const
    {
        return fwd_connector;
    }

    Connector *getBwdConnector() const
    {
        return bwd_connector;
    }

    void setBusy(bool is_busy)
    {
        this->is_busy = is_busy;
    }

    bool isBusy() const
    {
        return is_busy;
    }

    /// Вернуть первый трек траектории
    track_t getFirstTrack() const
    {
        return *tracks.begin();
    }

    /// Вернуть последний трек траектории
    track_t getLastTrack() const
    {
        return *(tracks.end() - 1);
    }

private:

    QString name = "";

    double len = 0.0;

    bool is_busy = false;

    Connector *fwd_connector = Q_NULLPTR;

    Connector *bwd_connector = Q_NULLPTR;

    std::vector<track_t>    tracks;

    /// Поиск текущего и следующего трека
    track_t findTracks(double traj_coord, track_t &next_track);
};

#endif
