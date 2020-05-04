#ifndef     SWITCH_H
#define     SWITCH_H

#include    "connector.h"

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

    Trajectory *fwdMinusTraj;

    Trajectory *fwdPlusTraj;

    Trajectory *bwdMinusTraj;

    Trajectory *bwdPlusTraj;
};

#endif // SWITCH_H
