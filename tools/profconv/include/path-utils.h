//------------------------------------------------------------------------------
//
//      Function for cross palform work with file paths
//      (c) maisvendoo, 20/12/2018
//
//------------------------------------------------------------------------------
#ifndef     PATH_UTILS_H
#define     PATH_UTILS_H

#include    <string>
#include    <istream>

std::string toNativeSeparators(const std::string &path);

char separator();

std::string compinePath(const std::string &path1, const std::string &path2);

std::string delete_symbol(const std::string &str, char symbol);

std::string getLine(std::istream &stream);

#endif // PATH_UTILS_H
