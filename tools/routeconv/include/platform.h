#ifndef     PLATFORM_H
#define     PLATFORM_H

#include    <QString>

const QString PATHCONV = "pathconv";
const QString PROFCONV = "profconv";
const QString PARALLELGEN = "offset-parallel-gen";
const QString SPLINEGEN = "offset-spline-gen";

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

#endif // PLATFORM_H
