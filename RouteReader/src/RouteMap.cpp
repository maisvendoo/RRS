#include "RouteReader/RouteMap.h"

#include <Journal.h>

#include <QString>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

bool RouteMap::read(std::string_view path)
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
        std::replace(line.begin(), line.end(), ',', ' ');
        parse_line(line);
    }

    return true;
}

void RouteMap::parse_line(const std::string& line)
{
    std::istringstream iss{line};
    Label label{};
    Transform transform{};
    auto& pos = transform.pos;
    auto& rot_deg = transform.rot_deg;
    iss >> label >> pos.x >> pos.y >> pos.z >>
           rot_deg.x >> rot_deg.y >> rot_deg.z;
    if (iss)
    {
        (*this)[label].emplace_back(transform);
    }
}
