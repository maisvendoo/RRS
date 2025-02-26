#ifndef     SOUND_EXPORT_H
#define     SOUND_EXPORT_H

#include    <QtGlobal>

#if defined(SOUND_MANAGER_LIB)
    #define SOUND_MANAGER_EXPORT    Q_DECL_EXPORT
#else
    #define SOUND_MANAGER_EXPORT    Q_DECL_IMPORT
#endif

#endif // SOUND_EXPORT_H
