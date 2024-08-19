#ifndef     SWITCH_H
#define     SWITCH_H

#include    <connector.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Switch : public Connector
{
    Q_OBJECT

public:

    Switch(QObject *parent = Q_NULLPTR);

    ~Switch();    

    Trajectory *getFwdTraj() const;

    Trajectory *getBwdTraj() const;

    void configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list);

    QByteArray serialize() override;

private:

    Trajectory *fwdMinusTraj = Q_NULLPTR;

    Trajectory *fwdPlusTraj = Q_NULLPTR;

    Trajectory *bwdMinusTraj = Q_NULLPTR;

    Trajectory *bwdPlusTraj = Q_NULLPTR;

    /// Состояние стрелки впереди
    int state_fwd = 1;

    /// Состояние стрелки сзади
    int state_bwd = 1;
};

#endif // SWITCH_H
