#ifndef CORE_LOAD_MODULE_H
#define CORE_LOAD_MODULE_H

#include <string>
#include <string_view>

class QString;

#define LOAD_MODULE(ClassType, lib_path)    \
    (ClassType*)load_module(lib_path)

using GetModuleFuncPtr = void* (*)();

GetModuleFuncPtr    load_get_module_func(const char* lib_path);
GetModuleFuncPtr    load_get_module_func(const std::string& lib_path);
GetModuleFuncPtr    load_get_module_func(std::string_view lib_path);
GetModuleFuncPtr    load_get_module_func(const QString& lib_path);

void*               load_module(const char* lib_path);
void*               load_module(const std::string& lib_path);
void*               load_module(std::string_view lib_path);
void*               load_module(const QString& lib_path);

#endif // CORE_LOAD_MODULE_H
