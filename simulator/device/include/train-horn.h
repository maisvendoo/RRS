#ifndef     TRAIN_HORN_H
#define     TRAIN_HORN_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT TrainHorn : public Device
{
public:

    TrainHorn(QObject *parent = Q_NULLPTR);

    virtual ~TrainHorn();

protected:

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);
};

#endif // TRAIN_HORN_H
