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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getLine(std::istream &stream)
{
    std::string line = "";
    std::getline(stream, line);
    std::string tmp = delete_symbol(line, '\r');
    tmp = delete_symbol(tmp, ';');
    std::replace(tmp.begin(), tmp.end(), ',', ' ');

    return tmp;
}
