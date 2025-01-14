#ifndef     DEVICE_EXPORT_H
#define     DEVICE_EXPORT_H

#if defined(WIN32)
#if defined(DEVICE_LIB)
    #define DEVICE_EXPORT   Q_DECL_EXPORT
#else
    #define DEVICE_EXPORT   Q_DECL_IMPORT
#endif
#else
    #define DEVICE_EXPORT
#endif

#endif // DEVICE_EXPORT_H
