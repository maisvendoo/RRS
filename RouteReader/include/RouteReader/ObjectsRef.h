#ifndef ROUTE_READER_OBJECTS_REF_H
#define ROUTE_READER_OBJECTS_REF_H

#include <map>
#include <string>
#include <string_view>

class ObjectsRef : public std::map<std::string, std::string>
{
public:
    using Label = std::string;
    using RelPath = std::string;

    bool read(std::string_view path);

private:
    void parse_line(const std::string& line);
};

#endif // ROUTE_READER_OBJECTS_REF_H
