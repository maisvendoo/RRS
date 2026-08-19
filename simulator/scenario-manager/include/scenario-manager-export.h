#ifndef     SCENARIO_MANAGER_EXPORT_H
#define     SCENARIO_MANAGER_EXPORT_H

#if defined(SCNMGR_LIB)
    #define SCNMGR_EXPORT Q_DECL_EXPORT
#else
    #define SCNMGR_EXPORT Q_DECL_IMPORT
#endif

#endif
