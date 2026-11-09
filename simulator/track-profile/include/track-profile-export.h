//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Export macro
//
//------------------------------------------------------------------------------

#ifndef     TRACK_PROFILE_EXPORT_H
#define     TRACK_PROFILE_EXPORT_H

#include    <QtGlobal>

#if defined(TRACKPROFILE_LIB)
    #define TRACKPROFILE_EXPORT  Q_DECL_EXPORT
#else
    #define TRACKPROFILE_EXPORT  Q_DECL_IMPORT
#endif

#endif // TRACK_PROFILE_EXPORT_H
