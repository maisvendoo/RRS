#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>
#include    <QMap>

#include    <topology-export.h>
#include    <track.h>
#include    <profile-point.h>

class Connector;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Trajectory : public QObject
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

    void setBusy(size_t idx, double coord);

    void setBusyState(bool busy_state);

    bool isBusy() const;

    bool isBusy(double coord_begin, double coord_end) const;

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

    /// Получить положение ПЕ на траектории
    profile_point_t getPosition(double traj_coord, int direction);

    /// Шаг симуляции
    virtual void step(double t, double dt);

    QByteArray serialize();

    void deserialize(QByteArray &data);

    std::vector<track_t> getTracks()
    {
        return tracks;
    }

signals:

    void sendTrajBusyState(QByteArray busy_data);

private:

    QString name = "";

    double len = 0.0;

    bool is_busy = false;

    QMap<size_t, double> vehicles_coords;

    Connector *fwd_connector = Q_NULLPTR;

    Connector *bwd_connector = Q_NULLPTR;

    std::vector<track_t>    tracks;

    /// Поиск текущего и следующего трека
    void findTracks(double traj_coord,
                    track_t &cur_track,
                    track_t &prev_track,
                    track_t &next_track);

    double calc_curvature(track_t &cur_track, track_t &next_track);
};

#endif
