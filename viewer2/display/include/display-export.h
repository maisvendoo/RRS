#ifndef     DISPLAY_EXPORT_H
#define     DISPLAY_EXPORT_H

#include    <QtGlobal>

#if defined(DISPLAY_LIB)
    #define DISPLAY_EXPORT   Q_DECL_EXPORT
#else
    #define DISPLAY_EXPORT   Q_DECL_IMPORT
#endif

#endif // DISPLAY_EXPORT_H
