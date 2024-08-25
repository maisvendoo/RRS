#ifndef     TRAJECTORYDEVICE_H
#define     TRAJECTORYDEVICE_H

#include    <QObject>
#include    <QMap>
#include    <device.h>

class ConnectorDevice;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TrajectoryDevice : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    TrajectoryDevice(QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~TrajectoryDevice();

    void setFwdConnectorDevice(ConnectorDevice *conn_device);
    void setBwdConnectorDevice(ConnectorDevice *conn_device);

    ConnectorDevice *getFwdConnectorDevice() const;
    ConnectorDevice *getBwdConnectorDevice() const;

    /// Set Device from Vehicle #idx
    void setLink(Device *device, size_t idx = 0);

    /// Шаг симуляции
    virtual void step(double t, double dt);

    /// Set name
    void setName(QString value);

    /// Get name
    QString getName() const;

    /// Set signal
    void setInputSignal(size_t idx, double value);

    /// Get signal
    double getOutputSignal(size_t idx) const;

    /// Read device config file
    virtual void read_config(const QString &filename, const QString &dir_path = "");
/*
    ///
    void setControl(control_signals_t control_signals = control_signals_t());

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

    ConnectorDevice *fwd_conn_device = nullptr;

    ConnectorDevice *bwd_conn_device = nullptr;

    QMap<size_t, Device *> vehicles_devices = {};

    /// Name of this device
    QString name = "";

    /// Input signals
    state_vector_t input_signals = {};
    /// Output signals
    state_vector_t output_signals = {};
/*
    control_signals_t   control_signals;

    feedback_signals_t  feedback;
*/
    /// Device configuration loading
    virtual void load_config(CfgReader &cfg);
};

#endif // TRAJECTORYDEVICE_H
