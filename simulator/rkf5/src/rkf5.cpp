#include    "rkf5.h"

#include	<cmath>

using namespace std;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RKF5Solver::RKF5Solver()
{
    MAX_ITER = 100;
    first_step = true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RKF5Solver::~RKF5Solver()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool RKF5Solver::step(OdeSystem *ode_sys,
                      state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t,
                      double &dt,
                      double max_step,
                      double local_err)
{
    size_t n = Y.size();

    // Share required memory
    if (first_step)
    {
        k1.resize(n);
        k2.resize(n);
        k3.resize(n);
        k4.resize(n);
        k5.resize(n);
        k6.resize(n);
        Y1.resize(n);
        eps_y.resize(n);

        first_step = false;
    }

    // Reset solver
    bool ready = false;
    int iter = 0;

    do
    {
        // Init local error
        double delta = 0;
        // Iteration count increment
        iter++;

        // Method step
        ode_sys->calcDerivative(Y, dYdt, t);        

        for (size_t i = 0; i < n; ++i)
        {
            k1[i] = dt * dYdt[i];
            Y1[i] = Y[i] + b21 * k1[i];
        }

        ode_sys->calcDerivative(Y1, dYdt, t + dt / 4.0);

        for (size_t i = 0; i < n; i++)
        {
            k2[i] = dt * dYdt[i];
            Y1[i] = Y[i] + b31 * k1[i] + b32 * k2[i];
        }

        ode_sys->calcDerivative(Y1, dYdt, t + 3.0 * dt / 8.0);

        for (size_t i = 0; i < n; i++)
        {
            k3[i] = dt * dYdt[i];
            Y1[i] = Y[i] + b41 * k1[i] + b42 * k2[i] + b43 * k3[i];
        }

        ode_sys->calcDerivative(Y1, dYdt, t + 12.0 * dt / 13.0);

        for (size_t i = 0; i < n; i++)
        {
            k4[i] = dt * dYdt[i];
            Y1[i] = Y[i] + b51 * k1[i] + b52 * k2[i] + b53 * k3[i] +
                b54 * k4[i];
        }

        ode_sys->calcDerivative(Y1, dYdt, t + dt);

        for (size_t i = 0; i < n; i++)
        {
            k5[i] = dt * dYdt[i];
            Y1[i] = Y[i] + b61 * k1[i] + b62 * k2[i] + b63 * k3[i] +
                b64 * k4[i] + b65 * k5[i];
        }

        ode_sys->calcDerivative(Y1, dYdt, t + dt / 2.0);

        // Local error calculation
        for (size_t i = 0; i < n; i++)
        {
            k6[i] = dt * dYdt[i];
            eps_y[i] = abs(e1 * k1[i] + e3 * k3[i] + e4 * k4[i] +
                e5 * k5[i] + e6 * k6[i]);

            // New state vector's value
            Y1[i] = Y[i] + c1 * k1[i] + c3 * k3[i] + c4 * k4[i] +
                    c5 * k5[i] + c6 * k6[i];

            if (delta < eps_y[i])
                delta = eps_y[i];
        }

        // Check error
        if (delta >= local_err)
        {
            dt = dt / 2;
            ready = false;
        }

        if (delta <= local_err / 32.0)
        {
            dt = 2 * dt;

            if (dt > max_step)
                dt = max_step;

            ready = true;
        }

        if ((delta > local_err / 32.0) && (delta < local_err))
        {
            ready = true;
        }


        // Store new value
        if (ready)
        {
            Y = Y1;
        }

    } while ((!ready) && (iter <= MAX_ITER));

    // Check interation count
    if (iter > MAX_ITER)
        return false;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_SOLVER(RKF5Solver)
