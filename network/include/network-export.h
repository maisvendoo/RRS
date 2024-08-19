#ifndef     NETWORK_EXPORT_H
#define     NETWORK_EXPORT_H

#include    <QtGlobal>

#if defined(NETWORK_LIB)
    #define NETWORK_EXPORT   Q_DECL_EXPORT
#else
    #define NETWORK_EXPORT   Q_DECL_IMPORT
#endif

#endif
