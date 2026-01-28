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
    QString getName() const { return this->name; }

    /// Получить оборудование путевой инфраструктуры на этом коннекторе
    const std::vector<ConnectorDevice *>& getConnectorDevices() const;

    /// Шаг симуляции
    virtual void step(double t, double dt);

    virtual QByteArray serialize();

    virtual void deserialize(QByteArray &data, traj_list_t &traj_list);

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

protected:

    Trajectory *fwdTraj = nullptr;

    Trajectory *bwdTraj = nullptr;

    int state = 1;

    QString name = "";

    /// Связи путевой инфраструктуры
    std::vector<ConnectorDevice *> devices;

    Signal *signal_fwd = nullptr;

    Signal *signal_bwd = nullptr;
};

#endif
