//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Export/import macros
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_EXPORT_H
#define     COLLISION_EXPORT_H

// Модуль чистый C++ (без Qt), поэтому обходимся без Q_DECL_EXPORT.
// Проверяем и _WIN32 (MSVC/MinGW), и __WIN32 (MinGW): общий заголовок
// import-export.h смотрит только на __WIN32, что под MSVC не работает
#if defined(_WIN32) || defined(__WIN32)
    #if defined(COLLISION_LIB)
        #define COLLISION_EXPORT  __declspec(dllexport)
    #else
        #define COLLISION_EXPORT  __declspec(dllimport)
    #endif
#else
    #define COLLISION_EXPORT
#endif

#endif // COLLISION_EXPORT_H
