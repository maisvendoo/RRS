//------------------------------------------------------------------------------
//
//      Function for cross palform work with file paths
//      (c) maisvendoo, 20/12/2018
//
//------------------------------------------------------------------------------
#ifndef     PATH_UTILS_H
#define     PATH_UTILS_H

#include    <string>

std::string toNativeSeparators(const std::string &path);

char separator();

std::string compinePath(const std::string &path1, const std::string &path2);

#endif // PATH_UTILS_H
