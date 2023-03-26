#include    "euler.h"

EulerSolver::EulerSolver()
{
    first_step = true;
}

EulerSolver::~EulerSolver()
{

}

bool EulerSolver::step(OdeSystem *ode_sys,
                     state_vector_t &Y,
                     state_vector_t &dYdt,
                     double t,
                     double &dt,
                     double max_step,
                     double local_err)
{
    Q_UNUSED(max_step)
    Q_UNUSED(local_err)

    // Share required memory
    if (first_step)
    {
        first_step = false;
    }

    ode_sys->calcDerivative(Y, dYdt, t, dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        Y[i] = Y[i] + dYdt[i] * dt;
    }

    return true;
}

GET_SOLVER(EulerSolver)
