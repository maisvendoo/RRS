//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     VERLET_H
#define     VERLET_H

#include    "solver.h"

class VerletSolver : public Solver
{
public:

    VerletSolver();

    bool step(OdeSystem *ode_sys,
              state_vector_t &Y,
              state_vector_t &dYdt,
              double t,
              double &dt,
              double max_step,
              double local_err);

    void init(size_t ode_order);

private:

    state_vector_t a;
    bool           first_step;
};

#endif // VERLET_H
