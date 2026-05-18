#include "core/load_module.h"

#include "core/string_funcs.h"

#include <QLibrary>

#define LOAD_GET_MODULE_FUNC_IMPLEMENTATION(StringType)           \
    GetModuleFuncPtr load_get_module_func(StringType lib_path)    \
    {                                                             \
        QLibrary lib{to_qstring(lib_path)};                       \
        if (!lib.load())                                          \
        {                                                         \
            return nullptr;                                       \
        }                                                         \
                                                                  \
        return (GetModuleFuncPtr)lib.resolve("get_module");       \
    }

#define LOAD_MODULE_IMPLEMENTATION(StringType)                                     \
    void* load_module(StringType lib_path)                                         \
    {                                                                              \
        const GetModuleFuncPtr get_module_func{load_get_module_func(lib_path)};    \
        if (!get_module_func)                                                      \
        {                                                                          \
            return nullptr;                                                        \
        }                                                                          \
                                                                                   \
        return get_module_func();                                                  \
    }

LOAD_GET_MODULE_FUNC_IMPLEMENTATION(const char*)
LOAD_GET_MODULE_FUNC_IMPLEMENTATION(const std::string&)
LOAD_GET_MODULE_FUNC_IMPLEMENTATION(std::string_view)
LOAD_GET_MODULE_FUNC_IMPLEMENTATION(const QString&)

LOAD_MODULE_IMPLEMENTATION(const char*)
LOAD_MODULE_IMPLEMENTATION(const std::string&)
LOAD_MODULE_IMPLEMENTATION(std::string_view)
LOAD_MODULE_IMPLEMENTATION(const QString&)
