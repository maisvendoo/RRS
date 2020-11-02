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

/*!
 * \typedef
 * \brief ODE system state vector
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef     std::vector<double> state_vector_t;

Q_DECLARE_METATYPE(state_vector_t)

#endif // SOLVERTYPES_H
