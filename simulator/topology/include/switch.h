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

private:

    Trajectory *fwdMinusTraj = Q_NULLPTR;

    Trajectory *fwdPlusTraj = Q_NULLPTR;

    Trajectory *bwdMinusTraj = Q_NULLPTR;

    Trajectory *bwdPlusTraj = Q_NULLPTR;
};

#endif // SWITCH_H
