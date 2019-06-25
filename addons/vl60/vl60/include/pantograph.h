#ifndef     PANTOGRAPH_H
#define     PANTOGRAPH_H

#include    "device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Pantograph : public Device
{
public:

    Pantograph(QObject *parent = Q_NULLPTR);

    ~Pantograph();

private:

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);
};

#endif // PANTOGRAPH_H
