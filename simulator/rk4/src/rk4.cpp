#include    "rk4.h"

RK4Solver::RK4Solver()
{
    first_step = true;
}

RK4Solver::~RK4Solver()
{

}

bool RK4Solver::step(OdeSystem *ode_sys,
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
        k2.resize(n);
        k3.resize(n);
        //k4.resize(n);
        Y1.resize(n);

        first_step = false;
    }

    ode_sys->calcDerivative(Y, dYdt, t, dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        k1[i] = dYdt[i];
        Y1[i] = Y[i] + k1[i] * dt / 2.0;
    }

    ode_sys->calcDerivative(Y1, dYdt, t + dt / 2.0, dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        k2[i] = dYdt[i];
        Y1[i] = Y[i] + k2[i] * dt / 2.0;
    }

    ode_sys->calcDerivative(Y1, dYdt, t + dt / 2.0, dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        k3[i] = dYdt[i];
        Y1[i] = Y[i] + k3[i] * dt;
    }

    ode_sys->calcDerivative(Y1, dYdt, t + dt, dt);

    for (size_t i = 0; i < Y.size(); ++i)
    {
        //k4[i] = dYdt[i];
        //Y[i] = Y[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * dt / 6.0;
        Y[i] = Y[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + dYdt[i]) * dt / 6.0;
    }

    return true;
}

GET_SOLVER(RK4Solver)
