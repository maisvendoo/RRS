#ifndef     SWITCH_H
#define     SWITCH_H

#include    <connector.h>
#include    <cstdint>

class Signal;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Switch : public Connector
{
    Q_OBJECT

public:

    Switch(QObject *parent = nullptr);

    ~Switch();

    Trajectory *getFwdTraj() const override;

    Trajectory *getBwdTraj() const override;

    void configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list) override;

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

    QByteArray serialize() override;

    void deserialize(QByteArray &data, traj_list_t &traj_list) override;

    enum State : int8_t {
        STATE_MINUS = -1,       ///< Стрелка в минусовом положении (на бок)
        STATE_PLUS = 1,         ///< Стрелка в плусовом положении (прямо)
        IS_BUSY_MINUS = -2,     ///< Стрелка занята ПЕ в минусовом положении
        IS_BUSY_PLUS = 2,       ///< Стрелка занята ПЕ в плюсовом положении
        IN_ROUTE_MINUS = -3,    ///< Стрелка в маршруте в минусовом положении
        IN_ROUTE_PLUS = 3,      ///< Стрелка в маршруте в плюсовом положении
        ONE_POSSIBLE_DIRECTION = 0  ///< Единственная возможная траектория
    };

    State getStateFwd() const
    {
        return state_fwd;
    }

    State getStateBwd() const
    {
        return state_bwd;
    }

    void setStateFwd(State state);

    void setStateBwd(State state);

    void setRefStateFwd(State state);

    void setRefStateBwd(State state);

    Trajectory *fwdMinusTraj = nullptr;

    Trajectory *fwdPlusTraj = nullptr;

    Trajectory *bwdMinusTraj = nullptr;

    Trajectory *bwdPlusTraj = nullptr;

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

    /// Состояние стрелки впереди
    State state_fwd = ONE_POSSIBLE_DIRECTION;

    /// Состояние стрелки сзади
    State state_bwd = ONE_POSSIBLE_DIRECTION;

    /// Требуемое состояние стрелки впереди:
    State ref_state_fwd = STATE_PLUS;

    /// Требуемое состояние стрелки сзади:
    State ref_state_bwd = STATE_PLUS;

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
};

#endif // SWITCH_H
