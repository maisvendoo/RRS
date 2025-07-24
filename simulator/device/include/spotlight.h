#ifndef     SPOT_LIGHT_H
#define     SPOT_LIGHT_H

#include    <device.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT SpotLight : public Device
{
public:

    SpotLight();

    ~SpotLight();

private:

    virtual void preStep(state_vector_t &Y, double t);

    virtual void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    virtual void load_config(CfgReader &cfg);
};

#endif // SPOT_LIGHT_H
