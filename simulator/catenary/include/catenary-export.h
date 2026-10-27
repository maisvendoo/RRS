//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Export macro
//
//------------------------------------------------------------------------------

#ifndef     CATENARY_EXPORT_H
#define     CATENARY_EXPORT_H

#include    <QtGlobal>

#if defined(CATENARY_LIB)
    #define CATENARY_EXPORT  Q_DECL_EXPORT
#else
    #define CATENARY_EXPORT  Q_DECL_IMPORT
#endif

#endif // CATENARY_EXPORT_H
