//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     SOLVER_TYPES_H
#define     SOLVER_TYPES_H

#include    <vector>
#include    <QMetaType>

#include    <state-vector-allocator.h>

/*!
 * \typedef
 * \brief ODE system state vector
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using state_vector_t = std::vector<double, StateVectorAllocator<double, 64>>;

Q_DECLARE_METATYPE(state_vector_t)

#endif // SOLVERTYPES_H
