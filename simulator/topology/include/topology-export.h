#ifndef     TOPOLOGY_EXPORT_H
#define     TOPOLOGY_EXPORT_H

#if defined(TOPOLOGY_LIB)
    #define TOPOLOGY_EXPORT     Q_DECL_EXPORT
#else
    #define TOPOLOGY_EXPORT     Q_DECL_IMPORT
#endif

#endif
