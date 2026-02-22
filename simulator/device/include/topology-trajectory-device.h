#ifndef     TRAJECTORYDEVICE_H
#define     TRAJECTORYDEVICE_H

#include    <QObject>
#include    <QMap>
#include    <device.h>
#include    <device-list.h>

class Trajectory;
class ConnectorDevice;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TrajectoryDevice : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    TrajectoryDevice(QObject *parent = nullptr);

    /// Destructor
    virtual ~TrajectoryDevice();

    void setTrajectory(Trajectory *traj);
    Trajectory *getTrajectory() const;

    void setConnectorDevice(ConnectorDevice *conn_device, std::int8_t dir);

    ConnectorDevice *getNextConnectorDevice(std::int8_t& dir);

    /// Set Device for next step
    void clearLinks();

    /// Set Device for next step
    void setLink(device_coord_t device, std::int8_t direction);

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

    /// Device configuration
    virtual void load_config(CfgReader &cfg);

protected:

    Trajectory *trajectory = nullptr;

    ConnectorDevice *fwd_conn_device = nullptr;

    ConnectorDevice *bwd_conn_device = nullptr;

    device_coord_list_t vehicles_devices = {};

    std::vector<std::int8_t> vehicles_devices_directions = {};

    /// Name of this device
    QString name = "";

    /// Input signals
    state_vector_t input_signals = {};
    /// Output signals
    state_vector_t output_signals = {};
};


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef TrajectoryDevice* (*GetTrajectoryDevice)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_TRAJECTORY_DEVICE(ClassName) \
extern "C" TrajectoryDevice *getTrajectoryDevice() \
{ \
        return new (ClassName) (); \
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT TrajectoryDevice *loadTrajectoryDevice(QString lib_path);

#endif // TRAJECTORYDEVICE_H
