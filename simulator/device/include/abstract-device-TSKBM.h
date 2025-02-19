#ifndef     ABSTRACTDEVICETSKBM_H
#define     ABSTRACTDEVICETSKBM_H

#include    <QObject>
#include    "device.h"
#include    "device-export.h"

class DEVICE_EXPORT AbstractDeviceTSKBM : public Device
{
    Q_OBJECT

public:

    explicit AbstractDeviceTSKBM(QObject *parent = nullptr);
    virtual ~AbstractDeviceTSKBM();

    void step(double t, double dt) override;

    /// Device model ODE system
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    /// Запуск таймера периодической проверки бдительности ТСКБМ
    virtual void startSafetyTimer() const = 0;

    /// Остановка таймера периодической проверки бдительности ТСКБМ
    virtual void stopSafetyTimer() const = 0;

 signals:

    void TSKBMVigilanceCheck();
    void TSKBMfailureEPK();
};

typedef AbstractDeviceTSKBM* (*GetPluginTSKBMDevice)();

#define GET_PLUGIN_TSKBM_DEVICE(ClassName) \
extern "C" Q_DECL_EXPORT AbstractDeviceTSKBM *getPluginTSKBMDevice() \
{ \
         return new (ClassName) (); \
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AbstractDeviceTSKBM *loadPluginTSKBMDevice(QString lib_path);

#endif // ABSTRACTDEVICETSKBM_H
