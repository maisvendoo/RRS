#ifndef     CONNECTORDEVICE_H
#define     CONNECTORDEVICE_H

#include    <QObject>
#include    "device.h"
#include    "topology-trajectory-device.h"

class Connector;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT ConnectorDevice : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    ConnectorDevice(QObject *parent = nullptr);

    /// Destructor
    virtual ~ConnectorDevice();

    void setConnector(Connector *conn);
    Connector *getConnector() const;

    virtual void setFwdTrajectoryDevice(TrajectoryDevice *traj_device);
    virtual void setBwdTrajectoryDevice(TrajectoryDevice *traj_device);

    virtual TrajectoryDevice *getFwdTrajectoryDevice() const;
    virtual TrajectoryDevice *getBwdTrajectoryDevice() const;

    /// Шаг симуляции
    virtual void step(double t, double dt);

    /// Set name
    void setName(QString value);

    /// Get name
    QString getName() const;

    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);

protected:

    Connector *connector = nullptr;

    TrajectoryDevice *fwd_traj_device = nullptr;

    TrajectoryDevice *bwd_traj_device = nullptr;

    /// Name of this device
    QString name = "";
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef ConnectorDevice* (*GetConnectorDevice)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_CONNECTOR_DEVICE(ClassName) \
extern "C" ConnectorDevice *getConnectorDevice() \
{ \
        return new (ClassName) (); \
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT ConnectorDevice *loadConnectorDevice(QString lib_path);

#endif // CONNECTORDEVICE_H
