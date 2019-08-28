#ifndef     PLATFORM_H
#define     PLATFORM_H

#include    <QString>

#ifdef QT_DEBUG
    const QString PATHCONV = "pathconv_d";
    const QString PROFCONV = "profconv_d";
#else
    const QString PATHCONV = "pathconv";
    const QString PROFCONV = "profconv";
#endif

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

#endif // PLATFORM_H
