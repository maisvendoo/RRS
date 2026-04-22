#ifndef     SWITCH_H
#define     SWITCH_H

//#include    <connector.h>
#include    "topology-export.h"
#include    "topology-defines.h"

#include    <QObject>
#include    <QDomNode>

class CfgReader;
class ConnectorDevice;
class Signal;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Switch : public QObject/* : public Connector*/
{
    Q_OBJECT

public:

    Switch(QObject *parent = nullptr);

    ~Switch();

    QString getName() const;

    /// Список траекторий, подключенных к данной стрелке
    Trajectory* trajectories[switch_ways_t.size()] = {nullptr, nullptr, nullptr, nullptr};

    /// Ориентация подключения данной траектории к стрелке
    dir_t getTrajOrientation(const Trajectory* traj);

    /// Следующая траектория в данном направлении
    Trajectory* getNextTraj(dir_t& dir) const;

    Trajectory* get_fwd_minus_traj() const;
    Trajectory* get_fwd_plus_traj() const;
    Trajectory* get_bwd_minus_traj() const;
    Trajectory* get_bwd_plus_traj() const;

    void setSignalFwd(Signal *signal);
    Signal *getSignalFwd();
    const Signal* getSignalFwd() const;

    void setSignalBwd(Signal *signal);
    Signal *getSignalBwd();
    const Signal* getSignalBwd() const;

    void configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list);

    /// Шаг симуляции
    virtual void step(double t, double dt);

    QByteArray serialize() const;

    void deserialize(QByteArray &data, traj_list_t &traj_list);

    Switch_state_t getStateFwd() const;

    Switch_state_t getStateBwd() const;

    void setStateFwd(Switch_state_t state);

    void setStateBwd(Switch_state_t state);

    void setRefStateFwd(Switch_state_t state);

    void setRefStateBwd(Switch_state_t state);

    /// Светофор, включающий данный стрелочный перевод вперёд в маршрут ДЦ
    Signal* getRouteBySignalFwd() const;
    void setRouteBySignalFwd(Signal* signal);

    /// Светофор, включающий данный стрелочный перевод назад в маршрут ДЦ
    Signal* getRouteBySignalBwd() const;
    void setRouteBySignalBwd(Signal* signal);

signals:

    void sendSwitchState(QByteArray sw_data);

private:

    /// Ориентации траекторий (FWD - совпадает с ориентацией стрелки, BWD - противоположна)
    dir_t orientations[switch_ways_t.size()] = {FWD, FWD, FWD, FWD};

    /// Состояние стрелки впереди
    Switch_state_t state_fwd = NO_POSSIBLE_DIRECTION;

    /// Состояние стрелки сзади
    Switch_state_t state_bwd = NO_POSSIBLE_DIRECTION;

    /// Требуемое состояние стрелки впереди:
    Switch_state_t ref_state_fwd = NO_POSSIBLE_DIRECTION;

    /// Требуемое состояние стрелки сзади:
    Switch_state_t ref_state_bwd = NO_POSSIBLE_DIRECTION;

    QString name = "";

    /// Связи путевой инфраструктуры
    std::vector<ConnectorDevice *> devices;

    Signal *signal_fwd = nullptr;

    Signal *signal_bwd = nullptr;

    /// Стрелка будет заблокирована в сторону траектории,
    /// которая занята ПЕ ближе чем в 40 метрах
    const double lock_by_busy_distance = 40.0;

    /// Светофор, включающий данный стрелочный перевод вперёд в маршрут ДЦ
    Signal* in_route_by_signal_fwd = nullptr;
    /// Светофор, включающий данный стрелочный перевод назад в маршрут ДЦ
    Signal* in_route_by_signal_bwd = nullptr;

    void serialize_connected_trajectory(QDataStream &stream,
        Trajectory *traj, dir_t orient) const;

    std::pair<Trajectory*, dir_t> deserialize_connected_trajectory(QDataStream &stream,
                                                                   traj_list_t &traj_list);
};

#endif // SWITCH_H
