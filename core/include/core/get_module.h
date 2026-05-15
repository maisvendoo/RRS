#ifndef CORE_GET_MODULE_H
#define CORE_GET_MODULE_H

#include "core/module_export.h"

#define GET_MODULE(ClassName)                      \
    extern "C" MODULE_EXPORT void* get_module()    \
    {                                              \
        return (void*)(new (ClassName));           \
    }

#endif // CORE_GET_MODULE_H
