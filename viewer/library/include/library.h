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

#ifndef         LIBRARY_H
#define         LIBRARY_H

#if __unix__
    #include    <dlfcn.h>
#else
    #include   <windows.h>
#endif

#include        <string>
#include        "import-export.h"

#ifdef LIBRARY_LIB
    #define LIBRARY_EXPORT DECL_EXPORT
#else
    #define LIBRARY_EXPORT DECL_IMPORT
#endif

/*!
 * \class
 * \brief Library loader
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LIBRARY_EXPORT Library
{
public:

    /// Constructor
    Library(const std::string &path, const std::string &name);

    /// Destructor
    ~Library();

    /// Load library
    bool load();

    /// Get function address by name
    void *resolve(std::string func_name);

private:

    /// Path to library
    std::string _path;
    /// Pointer to library
    void        *_lib_ptr;
};

#endif // LIBRARY_H
