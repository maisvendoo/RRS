#include    "verlet.h"

VerletSolver::VerletSolver()
    : first_step(true)
{

}

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

    ode_sys->calcDerivative(Y, dYdt, a, t, dt);

    double half_sqr_dt = dt * dt / 2.0;

    for (size_t i = 0; i < Y.size(); i++)
    {
        dYdt[i] += a[i] * dt;
        Y[i] += dYdt[i] * dt + a[i] * half_sqr_dt;
    }

    return true;
}

void VerletSolver::init(size_t ode_order)
{
    a.resize(ode_order);
}

GET_SOLVER(VerletSolver)
