#ifndef     IO_CONTROLLER_EXPORT_H
#define     IO_CONTROLLER_EXPORT_H

#include    <QtGlobal>

#ifdef  IO_CONTROLLER_LIB
    #define IO_CONTROLLER_EXPORT Q_DECL_EXPORT
#else
    #define IO_CONTROLLER_EXPORT Q_DECL_IMPORT
#endif

#endif
