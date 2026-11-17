#include "RouteReader/ObjectsRef.h"

#include <Journal.h>

#include <QString>

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

static bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

bool ObjectsRef::read(std::string_view path)
{
    std::ifstream file{std::string{path}};
    if (!file)
    {
        Journal::instance()->error(QString{"Failed to open %1"}.arg(path));
        return false;
    }

    std::string line{};
    while (std::getline(file, line))
    {
        parse_line(line);
    }

    return true;
}

void ObjectsRef::parse_line(const std::string& line)
{
    std::istringstream iss{line};
    Label label{};
    RelPath rel_path{};
    iss >> label >> rel_path;
    if (!rel_path.empty() && is_slash(rel_path.front()))
    {
        this->emplace(std::move(label), std::move(rel_path));
    }
}
