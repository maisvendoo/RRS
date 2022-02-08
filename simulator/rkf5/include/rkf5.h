#ifndef     RKF5_H
#define     RKF5_H

#include    "solver.h"

/*!
 *  \class
 *  \brief 5-6 order Runge-Kutta's integration method
 */
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class  RKF5Solver : public Solver
{
public:

    RKF5Solver();
    ~RKF5Solver();

    /// Method step
    bool step(OdeSystem *ode_sys,
              state_vector_t &Y,
              state_vector_t &dYdt,
              double t,
              double &dt,
              double max_step,
              double local_err);

protected:

    // Constants
    const double c1 = 16.0 / 135;
    const double c3 = 6656.0 / 12825;
    const double c4 = 28561.0 / 56430;
    const double c5 = -9.0 / 50;
    const double c6 = 2.0 / 55;

    const double b21 = 0.25;

    const double b31 = 3.0 / 32;
    const double b32 = 9.0 / 32;

    const double b41 = 1932.0 / 2197;
    const double b42 = -7200.0 / 2197;
    const double b43 = 7296.0 / 2197;

    const double b51 = 439.0 / 216;
    const double b52 = -8.0;
    const double b53 = 3680.0 / 513;
    const double b54 = -845.0 / 4140;

    const double b61 = -8.0 / 27;
    const double b62 = 2.0;
    const double b63 = -3544.0 / 2565;
    const double b64 = 1859.0 / 4104;
    const double b65 = -11.0 / 40;

    const double e1 = 1.0 / 360;
    const double e3 = -128.0 / 4275;
    const double e4 = -2197.0 / 75240;
    const double e5 = 1.0 / 50;
    const double e6 = 2.0 / 55;

    state_vector_t k1;
    state_vector_t k2;
    state_vector_t k3;
    state_vector_t k4;
    state_vector_t k5;
    state_vector_t k6;
    state_vector_t Y1;

    // Local error
    state_vector_t eps_y;

    // First step flag
    bool first_step;

    // Maximal iteraion count
    int MAX_ITER;
};


#endif // RKF5_H
