#include    "verlet.h"

bool VerletSolver::step(OdeSystem *ode_sys,
                      state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t,
                      double &dt,
                      double max_step,
                      double local_err)
{
    (void) max_step;
    (void) local_err;

    state_vector_t a;
    a.resize(Y.size());

    ode_sys->calcDerivative(Y, dYdt, a, t, dt);

    for (size_t i = 0; i < Y.size(); i++)
    {
        Y[i] += dYdt[i] * dt + a[i] * dt * dt / 2.0;
        dYdt[i] += a[i] * dt;
    }

    return true;
}

GET_SOLVER(VerletSolver)
