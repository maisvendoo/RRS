#ifndef     CONNECTOR_H
#define     CONNECTOR_H

#include    <QObject>

#include    <trajectory.h>
#include    <topology-types.h>
#include    <topology-export.h>
#include    <CfgReader.h>
#include    <topology-connector-device.h>

class Signal;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Connector : public QObject
{
    Q_OBJECT

public:

    Connector(QObject *parent = nullptr);

    virtual ~Connector();

    virtual Trajectory *getFwdTraj() const { return fwdTraj; }

    virtual Trajectory *getBwdTraj() const { return bwdTraj; }

    virtual void setState(int state) { this->state = state; }

    virtual void configure(CfgReader &cfg,
                           QDomNode secNode,
                           traj_list_t &traj_list);
    /// Получить оборудование путевой инфраструктуры на этом коннекторе
    const std::vector<ConnectorDevice *>& getConnectorDevices() const;

    /// Шаг симуляции
    virtual void step(double t, double dt);

    virtual QByteArray serialize();

    virtual void deserialize(QByteArray &data, traj_list_t &traj_list);
protected:

    Trajectory *fwdTraj = nullptr;

    Trajectory *bwdTraj = nullptr;

    int state = 1;
};

#endif
