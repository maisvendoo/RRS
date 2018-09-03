//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     SOLVER_EXPORT_H
#define     SOLVER_EXPORT_H

#include    <QtGlobal>

#if defined(SOLVER_LIB)
    #define SOLVER_EXPORT   Q_DECL_EXPORT
#else
    #define SOLVER_EXPORT   Q_DECL_IMPORT
#endif

#endif // SOLVER_EXPORT_H
