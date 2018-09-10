#ifndef     PROFILE_EXPORT_H
#define     PROFILE_EXPORT_H

#include    <QtGlobal>

#if defined(PROFILE_LIB)
    #define PROFILE_EXPORT Q_DECL_EXPORT
#else
    #define PROFILE_EXPORT Q_DECL_IMPORT
#endif

#endif // PROFILE_EXPORT_H
