#ifndef     JOINT_H
#define     JOINT_H

#include    <QString>

#include    "device-export.h"
#include    "device-list.h"

#include "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Joint
{
public:

    /// Constructor
    Joint();
    /// Destructor
    virtual ~Joint();

    /// Read joint config file
    virtual void read_config(const QString &path);

    /// Set connector for linkage
    void setLink(Device *device, size_t idx = 0);

    /// Get linkage to connector
    void swapDevicesLinks(size_t idx1 = 0, size_t idx2 = 1);

    /// Simulation step
    virtual void step(double t, double dt);

    /// Сцепки соединены (диагностика продольной динамики)
    virtual bool isConnected() const { return false; }

    /// Соединение разрушено перегрузкой
    virtual bool isBroken() const { return false; }

    /// Текущее усилие в соединении, Н (положительное - растяжение)
    virtual double getForce() const { return 0.0; }

    /// Текущая относительная скорость элементов соединения, м/с
    virtual double getRelVelocity() const { return 0.0; }

    /// Повреждение соединения (0 - целое, 1 - разрушено)
    virtual double getDamage() const { return 0.0; }

protected:

    /// List of linked devices
    device_list_t devices;

    /// Joint configuration loading
    virtual void load_config(CfgReader &cfg);
};

#endif // JOINT_H
