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
    ConnectorDevice(QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~ConnectorDevice();

    void setConnector(Connector *conn);

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
/*
    ///
    void setControl(QMap<int, bool> keys,
                    control_signals_t control_signals = control_signals_t());

    ///
    feedback_signals_t getFeedback() const;
*/
/*
    /// Device's sound state
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Device's sound state (as a single float value, see common-headers/sound-signal.h)
    virtual float getSoundSignal(size_t idx = 0) const;
*/
protected:

    Connector *connector = nullptr;

    TrajectoryDevice *fwd_traj_device = nullptr;

    TrajectoryDevice *bwd_traj_device = nullptr;

    /// Name of this device
    QString name = "";
/*
    control_signals_t   control_signals;

    feedback_signals_t  feedback;
*/
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef ConnectorDevice* (*GetConnectorDevice)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_CONNECTOR_DEVICE(ClassName) \
extern "C" Q_DECL_EXPORT ConnectorDevice *getConnectorDevice() \
{ \
        return new (ClassName) (); \
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT ConnectorDevice *loadConnectorDevice(QString lib_path);

#endif // CONNECTORDEVICE_H
