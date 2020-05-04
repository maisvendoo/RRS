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

    void configure(QDomNode secNode);

    Trajectory *getFwdTraj() const;

    Trajectory *getBwdTraj() const;

private:

    Trajectory *fwdMinusTraj;

    Trajectory *fwdPlusTraj;

    Trajectory *bwdMinusTraj;

    Trajectory *bwdPlusTraj;
};

#endif // SWITCH_H
