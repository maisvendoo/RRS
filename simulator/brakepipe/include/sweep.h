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

#ifndef     SWEEP_H
#define     SWEEP_H

#include    <cstddef>

/*!
 * \fn
 * \brief Sweep method of linear equation system solve
 */
void sweep(size_t n, double *A, double *B, double *C, double *b, double *x);

#endif //SWEEP_H
