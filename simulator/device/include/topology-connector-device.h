#ifndef     CONNECTORDEVICE_H
#define     CONNECTORDEVICE_H

#include    <QObject>
#include    "device.h"

class Switch;
class TrajectoryDevice;

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

    void setConnector(Switch* conn);
    Switch* getConnector() const;

    void setTrajectoryDevice(TrajectoryDevice* traj_device, std::int8_t dir, std::int8_t orient);

    TrajectoryDevice* getTrajectoryDevice(std::int8_t& dir) const;
    std::int8_t getDeviceOrientation(TrajectoryDevice* traj_device) const;

    /// Шаг симуляции
    virtual void step(double t, double dt);

    /// Set name
    void setName(QString value);

    /// Get name
    QString getName() const;

    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);

protected:

    Switch* connector = nullptr;

    TrajectoryDevice* fwd_traj_device = nullptr;
    TrajectoryDevice* bwd_traj_device = nullptr;

    /// Orientation of forward trajectory: 1 - co-directional, -1 - reversed
    std::int8_t fwd_dir = 1;
    /// Orientation of backward trajectory: 1 - co-directional, -1 - reversed
    std::int8_t bwd_dir = 1;

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
