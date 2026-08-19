#include    "euler.h"

#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EulerSolver::EulerSolver()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EulerSolver::~EulerSolver()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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

    ode_sys->calcDerivative(Y, dYdt, t, dt);

    // Помещаем в регистры все используемые в цикле параметры
    const double h = dt;
    const size_t n = Y.size();
    double* __restrict__ y = Y.data();
    const double* __restrict__ dydt = dYdt.data();

    #pragma GCC ivdep
    #pragma GCC unroll 4
    for (size_t i = 0; i < n; ++i)
    {
        y[i] = y[i] + dydt[i] * h;
    }

    return true;
}

GET_MODULE(EulerSolver)
