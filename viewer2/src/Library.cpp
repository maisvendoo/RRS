#include "Library.h"
#include "filesystem.h"

#ifdef __unix__
    #include <dlfcn.h>
    #define LIBRARY_EXTENSION ".so"
    #define LOAD_LIBRARY(path) dlopen((path).c_str(), RTLD_LAZY)
    #define UNLOAD_LIBRARY(ptr) dlclose((ptr))
    #define RESOLVE(ptr, func_name) dlsym((ptr), (func_name).c_str())
#else
    #include <windows.h>
    #define LIBRARY_EXTENSION ".dll"
    #define LOAD_LIBRARY(path) LoadLibraryA((path).c_str())
    #define UNLOAD_LIBRARY(ptr) FreeLibrary((HMODULE)(ptr))
    #define RESOLVE(ptr, func_name) GetProcAddress((HMODULE)(ptr), (func_name).c_str())
#endif

Library::Library(const std::string& path, const std::string& name)
    : path("")
    , lib_ptr(nullptr)
{
    FileSystem& fs = FileSystem::getInstance();
    this->path = fs.getNativePath(path) + fs.separator() + name + LIBRARY_EXTENSION;
}

Library::~Library()
{
    if (lib_ptr)
    {
        UNLOAD_LIBRARY(lib_ptr);
    }
}

bool Library::load()
{
    if (path.empty())
    {
        return false;
    }

    lib_ptr = LOAD_LIBRARY(path);

    return lib_ptr ? true : false;
}

void* Library::resolve(const std::string& func_name)
{
    return (void*)RESOLVE(lib_ptr, func_name);
}

