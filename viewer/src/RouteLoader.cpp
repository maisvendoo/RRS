#include "RouteLoader.h"

#include "CfgReader.h"
#include "Logger.h"
#include "Route.h"
// #include "parse_file_funcs.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vsg/maths/vec3.h>

#define LABEL_BUFFER_SIZE 256
#define RELATIVE_PATH_BUFFER_SIZE 512
#define FLOAT_BUFFER_SIZE 32

bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader::RouteLoader(const std::string& route_path)
    : route_path(route_path)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteLoader::read_description()
{
    QString tmp_qstr = (route_path + "/description.xml").c_str();

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoader::parse_objects_ref(Route& route)
{
    // char label[LABEL_BUFFER_SIZE];
    // char relative_path[RELATIVE_PATH_BUFFER_SIZE];

    // return parse_file_line_by_line(
    //     (route_path + "/objects.ref").c_str(), "r", " \t\r,;",
    //     [&]() -> void {
    //         std::string model_path = relative_path;
    //         if (!model_path.empty() && is_slash(model_path.front()))
    //         {
    //             route.object_ref.insert({label, relative_path});
    //         }
    //     }, 2,
    //     PARSE_VALUE_TYPE_STRING, label, static_cast<std::size_t>(LABEL_BUFFER_SIZE),
    //     PARSE_VALUE_TYPE_STRING, relative_path, static_cast<std::size_t>(RELATIVE_PATH_BUFFER_SIZE)
    // );

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
            // std::replace(model_path.begin(), model_path.end(), '/', '\\');
            route.object_ref.insert({label, model_path});
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteLoader::parse_route_map(Route& route)
{
    // char label[LABEL_BUFFER_SIZE];
    // char float_buffer[FLOAT_BUFFER_SIZE];
    // vsg::vec3 translation;
    // vsg::vec3 rotation;

    // return parse_file_line_by_line(
    //     (route_path + "/topology/map/route1.map").c_str(), "r", " \t\r,;",
    //     [&]() -> void {
    //         route.route_map[label].emplace_back(RouteObjectTransform{
    //             translation.x, translation.y, translation.z,
    //             rotation.x, rotation.y, rotation.z
    //         });
    //     }, 7,
    //     PARSE_VALUE_TYPE_STRING, label, static_cast<std::size_t>(LABEL_BUFFER_SIZE),
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.x,
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.y,
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &translation.z,
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.x,
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.y,
    //     PARSE_VALUE_TYPE_FLOAT, float_buffer, static_cast<std::size_t>(FLOAT_BUFFER_SIZE), &rotation.z
    // );

    std::ifstream route_map(route_path + "/topology/map/route1.map");
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
        vsg::vec3 translation;
        vsg::vec3 rotation_deg;
        line_stream >> label >> translation.x >> translation.y >> translation.z
            >> rotation_deg.x >> rotation_deg.y >> rotation_deg.z;
        if (line_stream)
        {
            route.route_map[label].emplace_back(RouteObjectTransform{translation, rotation_deg});
        }
    }

    return true;
}

