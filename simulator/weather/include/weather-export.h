//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Export macro
//
//------------------------------------------------------------------------------

#ifndef     WEATHER_EXPORT_H
#define     WEATHER_EXPORT_H

#include    <QtGlobal>

#if defined(WEATHER_LIB)
    #define WEATHER_EXPORT  Q_DECL_EXPORT
#else
    #define WEATHER_EXPORT  Q_DECL_IMPORT
#endif

#endif // WEATHER_EXPORT_H
