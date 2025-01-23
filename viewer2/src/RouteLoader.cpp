#include "RouteLoader.h"
#include "ConfigReader.h"
#include "Logger.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

RouteLoader::RouteLoader(const std::string& route_path)
    : route_path(route_path)
{
}

void RouteLoader::read_description()
{
    ConfigReader cfg(route_path + "/description.xml");
    cfg.setSection("Route");
    cfg.getValue("RouteType", route_type);
    cfg.getValue("ObjectsRefPath", objects_ref_path);
    cfg.getValue("RouteMapPath", route_map_path);
    objects_ref_path = route_path + objects_ref_path;
    route_map_path = route_path + route_map_path;

    int a = 10;
}

bool RouteLoader::parse_objects_ref(Route& route)
{
    std::ifstream objects_ref(objects_ref_path);
    if (!objects_ref)
    {
        LOG_ERROR("Failed to open %s", objects_ref_path.c_str());
        return false;
    }

    if (route_type == "dmd")
    {
        if (!std::filesystem::create_directory(route_path + "/united")
            && !std::filesystem::exists(route_path + "/united")
        )
        {
            return false;
        }

        std::string line;
        while (std::getline(objects_ref, line))
        {
            std::istringstream line_stream(line);
            std::string label;
            std::string model_path;
            std::string texture_path;
            line_stream >> label >> model_path >> texture_path;
            if (!texture_path.empty()
                && is_slash(model_path.front())
                && is_slash(texture_path.front())
            )
            {
                std::replace(model_path.begin(), model_path.end(), '\\', '/');
                std::replace(texture_path.begin(), texture_path.end(), '\\', '/');
                std::ofstream united_file(route_path + "/united/" + label + ".dmdu");
                united_file << "/.." << model_path << "\n/.." << texture_path << '\n';

            }
        }
    }
    else
    {
        std::string line;
        while (std::getline(objects_ref, line))
        {
            LOG_INFO("%s", line.c_str());
        }
    }

    return true;
}

bool RouteLoader::parse_route_map(Route& route)
{
    std::ifstream route_map(route_map_path);
    if (!route_map)
    {
        LOG_ERROR("Failed to open %s", route_map_path.c_str());
        return false;
    }

    return true;
}

