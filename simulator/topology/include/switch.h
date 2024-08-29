#ifndef     SWITCH_H
#define     SWITCH_H

#include    <connector.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Switch : public Connector
{
    Q_OBJECT

public:

    Switch(QObject *parent = Q_NULLPTR);

    ~Switch();

    Trajectory *getFwdTraj() const override;

    Trajectory *getBwdTraj() const override;

    void configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list) override;

    /// Шаг симуляции
    virtual void step(double t, double dt) override;

    QByteArray serialize() override;

    void deserialize(QByteArray &data, traj_list_t &traj_list) override;

    int getStateFwd() const
    {
        return state_fwd;
    }

    int getStateBwd() const
    {
        return state_bwd;
    }

    void setStateFwd(int state);

    void setStateBwd(int state);

    void setRefStateFwd(int state);

    void setRefStateBwd(int state);

signals:

    void sendSwitchState(QByteArray sw_data);

private:

    Trajectory *fwdMinusTraj = Q_NULLPTR;

    Trajectory *fwdPlusTraj = Q_NULLPTR;

    Trajectory *bwdMinusTraj = Q_NULLPTR;

    Trajectory *bwdPlusTraj = Q_NULLPTR;

    /// Состояние стрелки впереди: 0 - вперёд единственная траектория,
    /// >0 - в плюсовом положении, <0 - в минусовом положении,
    /// 1 - свободна, 2 - занята ПЕ
    int state_fwd = 0;

    /// Состояние стрелки сзади: 0 - назад единственная траектория,
    /// >0 - в плюсовом положении, <0 - в минусовом положении,
    /// 1 - свободна, 2 - занята ПЕ
    int state_bwd = 0;

    /// Требуемое состояние стрелки впереди:
    /// 1 - в плюсовом положении, -1 - в минусовом положении
    int ref_state_fwd = 1;

    /// Требуемое состояние стрелки сзади:
    /// 1 - в плюсовом положении, -1 - в минусовом положении
    int ref_state_bwd = 1;

    /// Стрелка будет заблокирована в сторону траектории,
    /// которая занята ПЕ ближе чем в 40 метрах
    const double lock_by_busy_distance = 40.0;

    void serialize_connected_trajectory(QDataStream &stream, Trajectory *traj);

    Trajectory *deserialize_connected_trajectory(QDataStream &stream,
                                          traj_list_t &traj_list);
};

#endif // SWITCH_H
