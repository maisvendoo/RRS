#ifndef     EULER2_H
#define     EULER2_H

#include    "solver.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class  Euler2Solver : public Solver
{
public:

    Euler2Solver();
    ~Euler2Solver();

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
    //state_vector_t k2;

    state_vector_t Y1;

    bool first_step;
};

#endif // EULER2_H
