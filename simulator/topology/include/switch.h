#ifndef     SWITCH_H
#define     SWITCH_H

#include    <QObject>

#include    <CfgReader.h>
#include    <topology-connector-device.h>

//#include    <connector.h>
#include    "topology-export.h"
#include    "topology-defines.h"

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

    QString getName() const { return this->name; }

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

    void setSignalFwd(Signal *signal)
    {
        this->signal_fwd = signal;
    }

    Signal *getSignalFwd()
    {
        return signal_fwd;
    }

    const Signal* getSignalFwd() const
    {
        return signal_fwd;
    }

    void setSignalBwd(Signal *signal)
    {
        this->signal_bwd = signal;
    }

    Signal *getSignalBwd()
    {
        return signal_bwd;
    }

    const Signal* getSignalBwd() const
    {
        return signal_bwd;
    }

    void configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list);

    /// Шаг симуляции
    virtual void step(double t, double dt);

    QByteArray serialize();

    void deserialize(QByteArray &data, traj_list_t &traj_list);

    Switch_state_t getStateFwd() const
    {
        return state_fwd;
    }

    Switch_state_t getStateBwd() const
    {
        return state_bwd;
    }

    void setStateFwd(Switch_state_t state);

    void setStateBwd(Switch_state_t state);

    void setRefStateFwd(Switch_state_t state);

    void setRefStateBwd(Switch_state_t state);

    /// Светофор, включающий данный стрелочный перевод вперёд в маршрут ДЦ
    Signal* getRouteBySignalFwd() const
    {
        return in_route_by_signal_fwd;
    }
    void setRouteBySignalFwd(Signal* signal)
    {
        in_route_by_signal_fwd = signal;
    }

    /// Светофор, включающий данный стрелочный перевод назад в маршрут ДЦ
    Signal* getRouteBySignalBwd() const
    {
        return in_route_by_signal_bwd;
    }
    void setRouteBySignalBwd(Signal* signal)
    {
        in_route_by_signal_bwd = signal;
    }

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

    void serialize_connected_trajectory(QDataStream &stream, Trajectory *traj);

    Trajectory *deserialize_connected_trajectory(QDataStream &stream,
                                          traj_list_t &traj_list);

    struct traj_xml_nodes_t
    {
        Switch_way_t way;
        QString normal_trajectory_node_name;
        QString reversed_trajectory_node_name;
        traj_xml_nodes_t(Switch_way_t w, QString n, QString r)
            : way(w)
            , normal_trajectory_node_name(n)
            , reversed_trajectory_node_name(r){}
    };
    inline static const traj_xml_nodes_t traj_xml_nodes[] =
    {
        traj_xml_nodes_t(SW_FWD_PLUS, QString("fwdPlusTraj"), QString("fwdPlusTrajReversed")),
        traj_xml_nodes_t(SW_FWD_MINUS, QString("fwdMinusTraj"), QString("fwdMinusTrajReversed")),
        traj_xml_nodes_t(SW_BWD_PLUS, QString("bwdPlusTraj"), QString("bwdPlusTrajReversed")),
        traj_xml_nodes_t(SW_BWD_MINUS, QString("bwdMinusTraj"), QString("bwdMinusTrajReversed"))
    };
};

#endif // SWITCH_H
