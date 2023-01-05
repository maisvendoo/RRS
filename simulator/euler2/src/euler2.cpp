#include    "euler2.h"

Euler2Solver::Euler2Solver()
{
    first_step = true;
}

Euler2Solver::~Euler2Solver()
{

}

bool Euler2Solver::step(OdeSystem *ode_sys,
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
        size_t n = Y.size();
        k1.resize(n);
        Y1.resize(n);

        first_step = false;
    }

    ode_sys->calcDerivative(Y, dYdt, t);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        k1[i] = dYdt[i];
        Y1[i] = Y[i] + k1[i] * dt;
    }

    ode_sys->calcDerivative(Y1, dYdt, t + dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        //k2[i] = dYdt[i];
        //Y[i] = Y[i] + (k1[i] + k2[i]) * dt / 2.0;
        Y[i] = Y[i] + (k1[i] + dYdt[i]) * dt / 2.0;
    }

    return true;
}

GET_SOLVER(Euler2Solver)
