//------------------------------------------------------------------------------
//
//      Conductors system for RRS
//      Export macro
//
//------------------------------------------------------------------------------

#ifndef     CONDUCTOR_EXPORT_H
#define     CONDUCTOR_EXPORT_H

#include    <QtGlobal>

#if defined(CONDUCTOR_LIB)
    #define CONDUCTOR_EXPORT  Q_DECL_EXPORT
#else
    #define CONDUCTOR_EXPORT  Q_DECL_IMPORT
#endif

#endif // CONDUCTOR_EXPORT_H
