#ifndef VIEWER_LIBRARY_H
#define VIEWER_LIBRARY_H

#include <string>

class Library
{
public:
    Library(const std::string& path, const std::string& name);

    ~Library();

    bool load();

    void* resolve(const std::string& func_name);

private:
    std::string path;
    void* lib_ptr;
};

#endif // VIEWER_LIBRARY_H
