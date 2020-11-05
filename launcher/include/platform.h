#ifndef     PLATFORM_H
#define     PLATFORM_H

#include    <QString>

#ifdef QT_DEBUG
    const QString SIMULATOR_NAME = "simulator_d";
    const QString VIEWER_NAME = "viewer_d";
#else
    const QString SIMULATOR_NAME = "simulator";
    const QString VIEWER_NAME = "viewer";
#endif

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

#endif // PLATFORM_H
