#ifndef     EULER_H
#define     EULER_H

#include    "solver.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class  EulerSolver : public Solver
{
public:

    EulerSolver();
    ~EulerSolver();

    /// Method step
    bool step(OdeSystem *ode_sys,
              state_vector_t &Y,
              state_vector_t &dYdt,
              double t,
              double &dt,
              double max_step,
              double local_err);

protected:

    bool first_step;
};

#endif // EULER_H
