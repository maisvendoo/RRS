#ifndef CORE_LOAD_MODULE_H
#define CORE_LOAD_MODULE_H

#include "core/string_funcs.h"

#include <QLibrary>

template <typename T, typename StringType>
T* (*load_get_module_func(const StringType& lib_path))()
{
    QLibrary lib{to_qstring(lib_path)};
    if (!lib.load())
    {
        return nullptr;
    }

    return (T* (*)())lib.resolve("get_module");
}

template <typename T, typename StringType>
T* load_module(const StringType& lib_path)
{
    const auto* get_module_func{load_get_module_func<T>(lib_path)};
    if (!get_module_func)
    {
        return nullptr;
    }

    return get_module_func();
}

#endif // CORE_LOAD_MODULE_H
