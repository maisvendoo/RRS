#ifndef CORE_MODULE_EXPORT_H
#define CORE_MODULE_EXPORT_H

#include <QtGlobal>

#if defined (WIN32)
    #if defined (MODULE_LIB)
        #define MODULE_EXPORT Q_DECL_EXPORT
    #else
        #define MODULE_EXPORT Q_DECL_IMPORT
    #endif
#else
    #define MODULE_EXPORT
#endif

#endif // CORE_MODULE_EXPORT_H
