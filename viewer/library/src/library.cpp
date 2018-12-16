//------------------------------------------------------------------------------
//
//      Cross-platform dynamic library loader
//      (c) maisvendoo, 03/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Cross-platform dynamic library loader
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 03/12/2018
 */

#include    "library.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Library::Library(const std::string &path, const std::string &name)
    : _path("")
    , _lib_ptr(nullptr)
{
    FileSystem &fs = FileSystem::getInstance();

    _path = fs.getNativePath(path) + fs.separator() + name;

#if __unix__
    _path += ".so";
#else
    _path += ".dll";
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Library::~Library()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Library::load()
{
    if (_path.empty())
        return false;

#if __unix__    

    _lib_ptr = dlopen(_path.c_str(), RTLD_LAZY);

    if (_lib_ptr != nullptr)
        return true;

#else

    _lib_ptr = LoadLibraryA(_path.c_str());

    if (_lib_ptr != nullptr)
        return true;
#endif

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void *Library::resolve(std::string func_name)
{
#if __unix__

    return dlsym(_lib_ptr, func_name.c_str());

#else

    return (void*) GetProcAddress((HMODULE) _lib_ptr, func_name.c_str());

#endif
}
