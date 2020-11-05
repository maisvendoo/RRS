#ifndef     RK4_H
#define     RK4_H

#include    "solver.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class  RK4Solver : public Solver
{
public:

    RK4Solver();
    ~RK4Solver();

    /// Method step
    bool step(OdeSystem *ode_sys,
              state_vector_t &Y,
              state_vector_t &dYdt,
              double t,
              double &dt,
              double max_step,
              double local_err);

protected:

    state_vector_t k1;
    state_vector_t k2;
    state_vector_t k3;
    state_vector_t k4;

    state_vector_t Y1;

    bool first_step;
};

#endif
