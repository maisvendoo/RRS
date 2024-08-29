#ifndef     CONNECTOR_H
#define     CONNECTOR_H

#include    <QObject>

#include    <trajectory.h>
#include    <topology-types.h>
#include    <topology-export.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Connector : public QObject
{
    Q_OBJECT

public:

    Connector(QObject *parent = Q_NULLPTR);

    virtual ~Connector();

    virtual Trajectory *getFwdTraj() const { return fwdTraj; }

    virtual Trajectory *getBwdTraj() const { return bwdTraj; }

    virtual void setState(int state) { this->state = state; }

    virtual void configure(CfgReader &cfg,
                           QDomNode secNode,
                           traj_list_t &traj_list);

    QString getName() const { return this->name; }

    /// Шаг симуляции
    virtual void step(double t, double dt);

    virtual QByteArray serialize();

    virtual void deserialize(QByteArray &data, traj_list_t &traj_list);

protected:

    Trajectory *fwdTraj = Q_NULLPTR;

    Trajectory *bwdTraj = Q_NULLPTR;

    int state = 1;

    QString name = "";
};

#endif
