//------------------------------------------------------------------------------
//
//      Sweep method of linear equation system solve
//      (с) maisvendoo, 06/11/2016
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Sweep method of linear equation system solve
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 06/11/2016
 */

#include "sweep.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void sweep(int n,      // system order
           double *A,  // Bottom diagonal
           double *B,  // Top diagonal
           double *C,  // Main diagonal
           double *f,  // Right part column
           double *x)  // Solution vector
{
    // Set zeros in bottom diagonal
    for (int i = 1; i < n; i++)
    {
        double m = A[i] / C[i - 1];

        C[i] = C[i] - m * B[i-1];
        f[i] = f[i] - m * f[i-1];
    }

    // Calcuation of solution
    x[n-1] = f[n-1] / C[n-1];

    for (int i = n-2; i >= 0; i--)
        x[i] = (f[i] - B[i] * x[i+1]) / C[i];
}
