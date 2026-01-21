#ifndef     PLATFORM_H
#define     PLATFORM_H

#include    <QString>

#define PATHCONV "pathconv"
#define PROFCONV "profconv"
#define DMD2GLTF "dmd2gltf"
#define TOPOLOGYCHECK "topologycheck"
#define PARALLELGEN "offset-parallel-gen"
#define SPLINEGEN "offset-spline-gen"

#ifdef __WIN32__
    #define EXE_EXP ".exe"
#else
    #define EXE_EXP ""
#endif

#endif // PLATFORM_H
