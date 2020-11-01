#ifndef     MSUT_H
#define     MSUT_H

#include    "device.h"

#include    "msut-data.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MSUT : public Device
{
public:

    MSUT(QObject *parent = Q_NULLPTR);

    ~MSUT();

private:

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // MSUT_H
