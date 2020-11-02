#include    "path-utils.h"

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string toNativeSeparators(const std::string &path)
{
    std::string tmp = path;

#ifdef __unix__
    std::replace(tmp.begin(), tmp.end(), '\\', '/');
#else
    std::replace(tmp.begin(), tmp.end(), '/', '\\');
#endif

    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
char separator()
{
#ifdef __unix__
    return '/';
#else
    return '\\';
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string compinePath(const std::string &path1, const std::string &path2)
{
    std::string tmp = "";

    if (*(path1.end() - 1) != separator())
        tmp = path1 + separator() + path2;
    else
        tmp = path1 + path2;

    return tmp;
}
