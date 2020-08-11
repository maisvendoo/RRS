#include    "file-funcs.h"

#include    <osgDB/FileUtils>
#include    <osgDB/FileNameUtils>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getFileName(const std::string &path)
{
    std::vector<std::string> pathElems;
    osgDB::getPathElements(path, pathElems);

    return *(pathElems.end() - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getFileNameInLowerCase(const std::string &path)
{
    return osgDB::convertToLowerCase(getFileName(path));
}
