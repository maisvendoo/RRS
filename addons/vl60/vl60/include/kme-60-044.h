#ifndef     KME_60_044_H
#define     KME_60_044_H

#include    "traction-controller.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControllerKME_60_044 : public TractionController
{
public:

    ControllerKME_60_044(QObject *parent);

    ~ControllerKME_60_044();

private:

    /// Положение главной рукоятки
    int     main_pos;

    /// Положение реверсивной рукоятки
    int     revers_pos;

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);
};

#endif // KME_60_044_H
