#ifndef     ABSTRACTBLOCKKON_H
#define     ABSTRACTBLOCKKON_H

#include    <QObject>
#include    "device.h"
#include    "device-export.h"

class DEVICE_EXPORT AbstractBlockKON : public Device
{

public:

    explicit AbstractBlockKON(QObject *parent = nullptr);
    virtual ~AbstractBlockKON();

    void step(double t, double dt) override;

    /// Device model ODE system
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t) override;

    virtual void setKeyEPK(bool key_epk) = 0;

    virtual void setVelocityKmh(double v_kmh) = 0;

    virtual void setBrakeCylinderPressure(double pBC) = 0;

    virtual bool getValveElectricalSupply() const = 0;
};

typedef AbstractBlockKON* (*GetPluginBlockKON)();

#define GET_PLUGIN_BLOCK_KON(ClassName) \
extern "C" Q_DECL_EXPORT AbstractBlockKON *getPluginBlockKON() \
{ \
        return new (ClassName) (); \
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT AbstractBlockKON *loadPluginBlockKON(QString lib_path);

#endif // ABSTRACTBLOCKKON_H
