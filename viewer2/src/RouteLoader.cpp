#include "RouteLoader.h"
#include "CfgReader.h"
#include "Logger.h"
#include "Route.h"
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
    QString tmp_qstr = (route_path + "\\description.xml").c_str();

    CfgReader cfg;
    cfg.load(tmp_qstr);

    QString sec_name = "Route";

    tmp_qstr = "";
    if (cfg.getString(sec_name, "RouteType", tmp_qstr))
        route_type = tmp_qstr.toStdString();

    tmp_qstr = "";
    if (cfg.getString(sec_name, "ObjectsRefPath", tmp_qstr))
        objects_ref_path = tmp_qstr.toStdString();

    tmp_qstr = "";
    if (cfg.getString(sec_name, "RouteMapPath", tmp_qstr))
        route_map_path = tmp_qstr.toStdString();

    objects_ref_path = route_path + objects_ref_path;
    route_map_path = route_path + route_map_path;
}

bool RouteLoader::parse_objects_ref(Route& route)
{
    std::ifstream objects_ref(route_path + "/objects.ref");
    if (!objects_ref)
    {
        LOG_ERROR("Failed to open %s", objects_ref_path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(objects_ref, line))
    {
        std::istringstream line_stream(line);
        std::string label;
        std::string model_path;
        line_stream >> label >> model_path;
        if (!model_path.empty()
            && is_slash(model_path.front()))
        {
            std::replace(model_path.begin(), model_path.end(), '/', '\\');
            route.object_ref.insert({label, model_path});
        }
    }

    return true;
}

bool RouteLoader::parse_route_map(Route& route)
{
    std::ifstream route_map(route_path + "/route1.map");
    if (!route_map)
    {
        LOG_ERROR("Failed to open %s", route_map_path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(route_map, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream line_stream(line);
        std::string label;
        float t_x, t_y, t_z;
        float r_x, r_y, r_z;
        line_stream >> label >> t_x >> t_y >> t_z >> r_x >> r_y >> r_z;
        if (line_stream)
        {
            route.transforms.insert({label, RouteObjectTransform{t_x, t_y, t_z, r_x, r_y, r_z}});
        }
    }

    return true;
}

