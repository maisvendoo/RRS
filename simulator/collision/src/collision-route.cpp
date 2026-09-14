//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Route files parser (objects.ref, topology/map/route1.map)
//
//------------------------------------------------------------------------------

#include    "collision-route.h"

#include    <algorithm>
#include    <cmath>
#include    <fstream>
#include    <sstream>

namespace collision
{

namespace
{

bool is_slash(char ch)
{
    return ch == '/' || ch == '\\';
}

//------------------------------------------------------------------------------
/// objects.ref: строки "label /относительный/путь/модели.gltf"
//------------------------------------------------------------------------------
bool parseObjectsRef(const std::string& path,
                     RouteObjectsData& out,
                     std::string* error)
{
    std::ifstream file(path);
    if (!file)
    {
        // Маршрут без objects.ref - не ошибка
        return true;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream line_stream(line);

        std::string label;
        std::string model_path;
        line_stream >> label >> model_path;

        if (!label.empty() && !model_path.empty() && is_slash(model_path.front()))
            out.models.insert({label, model_path});
        else if (!label.empty())
            ++out.skipped_lines;
    }

    (void) error;
    return true;
}

//------------------------------------------------------------------------------
/// route1.map: строки "label,x,y,z,rx,ry,rz;"
//------------------------------------------------------------------------------
bool parseRouteMap(const std::string& path,
                   RouteObjectsData& out,
                   std::string* error)
{
    std::ifstream file(path);
    if (!file)
    {
        // Маршрут без расстановки объектов - не ошибка
        return true;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream line_stream(line);

        RouteObjectInstance instance;
        line_stream >> instance.label
                    >> instance.position.x >> instance.position.y >> instance.position.z
                    >> instance.euler_deg.x >> instance.euler_deg.y >> instance.euler_deg.z;

        if (line_stream)
            out.instances.push_back(std::move(instance));
        else if (!instance.label.empty())
            ++out.skipped_lines;
    }

    (void) error;
    return true;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool loadRouteObjects(const std::string& route_dir,
                      RouteObjectsData& out,
                      std::string* error)
{
    if (!parseObjectsRef(route_dir + "/objects.ref", out, error))
        return false;

    if (!parseRouteMap(route_dir + "/topology/map/route1.map", out, error))
        return false;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Quatf quatFromRouteEuler(const Vec3f& euler_deg)
{
    constexpr float deg2rad = 3.14159265358979323846f / 180.0f;

    // Во вьювере углы инвертируются и применяются в порядке X, Y, Z:
    // R = Rz(-rz) * Ry(-ry) * Rx(-rx)
    const float hx = -0.5f * euler_deg.x * deg2rad;
    const float hy = -0.5f * euler_deg.y * deg2rad;
    const float hz = -0.5f * euler_deg.z * deg2rad;

    const Quatf qx(std::sin(hx), 0.0f, 0.0f, std::cos(hx));
    const Quatf qy(0.0f, std::sin(hy), 0.0f, std::cos(hy));
    const Quatf qz(0.0f, 0.0f, std::sin(hz), std::cos(hz));

    return qz * qy * qx;
}

} // namespace collision
