#ifndef     CONNECTOR_H
#define     CONNECTOR_H

#include    <QObject>

#include    "trajectory.h"
#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Connector : public QObject
{
    Q_OBJECT

public:

    Connector(QObject *parent = Q_NULLPTR);

    virtual ~Connector();

    Trajectory *getFwdTraj() const { return fwdTraj; }

    Trajectory *getBwdTraj() const { return bwdTraj; }

    void setState(int state) { this->state = state; }

    virtual void configure(QDomNode secNode) = 0;

protected:

    Trajectory *fwdTraj;

    Trajectory *bwdTraj;

    int state;
};

#endif // CONNECTOR_H
