#include    <filesystem-utils.h>
#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
char separator()
{
#if __unix__
    return '/';
#else
    return '\\';
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void path_to_native_separator(std::string &path)
{
    std::replace(path.begin(), path.end(), '\\', separator());
    std::replace(path.begin(), path.end(), '/', separator());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string combine_path(const std::string &path, const std::string &subpath)
{
    std::string result_path = path;
    path_to_native_separator(result_path);

    std::string tmp = subpath;
    path_to_native_separator(tmp);

    if ( result_path.back() != separator())
    {
        if (tmp.front() != separator())
        {
            result_path += separator();
        }
    }


    result_path += tmp;

    return result_path;
}
