#ifndef		IMPORT_EXPORT_H
#define		IMPORT_EXPORT_H

    #ifdef	__WIN32 || __WIN64
            #define DECL_EXPORT	__declspec(dllexport)
            #define DECL_IMPORT	__declspec(dllimport)
    #else
            #define DECL_EXPORT
            #define DECL_IMPORT
    #endif

#endif
