#ifndef     PLATFORM_H
#define     PLATFORM_H

#include    <QString>

    const QString SIMULATOR_NAME = "simulator";
    const QString VIEWER_NAME = "viewer";
    const QString ROUTE_MAP_NAME = "route-map";

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

#endif // PLATFORM_H
