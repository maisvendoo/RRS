//------------------------------------------------------------------------------
//
//      collider-gen: collision data generator for RRS routes
//
//      Читает модели маршрута (objects.ref + topology/map/route1.map),
//      извлекает геометрию из GLTF и генерирует colliders.conf:
//      - земля/поля      -> mesh (.colmesh рядом с моделями)
//      - высокие тонкие  -> cylinder (опоры и т.п.)
//      - остальные       -> box по AABB
//      - пути и провода  -> none (обрабатываются отдельными системами)
//
//      Существующие записи colliders.conf сохраняются (ручные правки),
//      если не передан --force.
//
//      Использование:
//          collider-gen <каталог_маршрута> [--force]
//
//------------------------------------------------------------------------------

#include    "mesh-extract.h"

#include    <collision-collider-config.h>
#include    <collision-layer.h>
#include    <collision-route.h>

#include    <vsg/maths/vec3.h>

#include    <algorithm>
#include    <cctype>
#include    <cstdio>
#include    <filesystem>
#include    <fstream>
#include    <iostream>
#include    <map>
#include    <sstream>
#include    <string>

namespace
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string fmt(float value)
{
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.6g", static_cast<double>(value));
    return buffer;
}

std::string fmtVec3(const collision::Vec3f& v)
{
    return fmt(v.x) + " " + fmt(v.y) + " " + fmt(v.z);
}

std::string toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return str;
}

//------------------------------------------------------------------------------
/// Эвристики: тип коллайдера, слой и профиль по метке и геометрии
//------------------------------------------------------------------------------
collision::ColliderEntry makeEntry(const std::string& label,
                                   const ExtractedMesh& geom)
{
    collision::ColliderEntry entry;

    const std::string low = toLower(label);

    // Пути и провода обрабатываются отдельными системами
    // (рельсы - из геометрии траекторий, контакт пантографа - отдельно)
    if (low.find("track") != std::string::npos ||
        low.find("wire") != std::string::npos)
    {
        entry.type = collision::ColliderType::None;
        return entry;
    }

    // Центр и полуразмеры AABB
    const vsg::dvec3 center = (geom.bounds.min + geom.bounds.max) * 0.5;
    const vsg::dvec3 extent = (geom.bounds.max - geom.bounds.min) * 0.5;

    entry.offset = collision::Vec3f(static_cast<float>(center.x),
                                    static_cast<float>(center.y),
                                    static_cast<float>(center.z));

    // Земля и поля - точный треугольный mesh
    if (low.find("floor") != std::string::npos ||
        low.find("field") != std::string::npos ||
        low.find("ground") != std::string::npos ||
        low.find("land") != std::string::npos)
    {
        entry.type = collision::ColliderType::Mesh;
        entry.offset = collision::Vec3f();  // mesh хранит мировые координаты вершин
        entry.layer = collision::Layer::Terrain;
        entry.profile = "terrain";
        return entry;
    }

    // Высокие тонкие объекты (опоры, столбы) - цилиндр
    if (extent.z > 2.0 * std::max(extent.x, extent.y) && extent.z > 1.0)
    {
        entry.type = collision::ColliderType::Cylinder;
        entry.half_height = static_cast<float>(extent.z);
        entry.radius = static_cast<float>(std::max(extent.x, extent.y));
        entry.layer = collision::Layer::Infrastructure;
        entry.profile = "concrete";
        return entry;
    }

    // Широкие протяжённые объекты (жёсткие поперечины, пролёты,
    // декорации во всю ширину станции): авто-AABB превращает их
    // в гигантские "невидимые стены", мешающие пешему режиму -
    // считаем декорациями без коллайдера (землю несёт mesh-слой)
    if (extent.x > 15.0 || extent.y > 15.0)
    {
        entry.type = collision::ColliderType::None;
        return entry;
    }

    // Придорожные объекты (знаки, сваи, мелкие строения): авто-AABB
    // в разы толще реальной опоры и залезает в габарит подвижного
    // состава. Сужаем поперечник бокса до стойки - вертикаль
    // (extent.z) и высота остаются настоящими
    const double max_xy = std::max(extent.x, extent.y);

    if (max_xy > 1.0)
    {
        entry.type = collision::ColliderType::Box;
        const double slim = std::min(0.35, max_xy);
        entry.half_extents = collision::Vec3f(static_cast<float>(slim),
                                              static_cast<float>(slim),
                                              static_cast<float>(extent.z));
        entry.layer = collision::Layer::Infrastructure;
        entry.profile = "default";
        return entry;
    }

    // Остальное - параллелепипед по AABB
    entry.type = collision::ColliderType::Box;
    entry.half_extents = collision::Vec3f(static_cast<float>(extent.x),
                                          static_cast<float>(extent.y),
                                          static_cast<float>(extent.z));
    entry.layer = collision::Layer::Infrastructure;
    entry.profile = "default";
    return entry;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string serializeEntry(const std::string& label,
                           const collision::ColliderEntry& entry)
{
    std::ostringstream out;
    out << label << ' ';

    switch (entry.type)
    {
    case collision::ColliderType::None:
        out << "none";
        return out.str();

    case collision::ColliderType::Box:
        out << "box " << fmtVec3(entry.half_extents);
        break;

    case collision::ColliderType::Sphere:
        out << "sphere " << fmt(entry.radius);
        break;

    case collision::ColliderType::Capsule:
        out << "capsule " << fmt(entry.half_height) << ' ' << fmt(entry.radius);
        break;

    case collision::ColliderType::Cylinder:
        out << "cylinder " << fmt(entry.half_height) << ' ' << fmt(entry.radius);
        break;

    case collision::ColliderType::Mesh:
        out << "mesh " << entry.file;
        break;
    }

    if (entry.offset.x != 0.0f || entry.offset.y != 0.0f || entry.offset.z != 0.0f)
        out << " offset " << fmtVec3(entry.offset);

    out << " layer " << collision::layerName(entry.layer)
        << " profile " << entry.profile;

    return out.str();
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: collider-gen <route_dir> [--force]" << std::endl;
        return 1;
    }

    const std::string route_dir = argv[1];

    bool force = false;
    for (int i = 2; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--force")
            force = true;
    }

    // Объекты маршрута
    collision::RouteObjectsData route;
    std::string error;
    if (!collision::loadRouteObjects(route_dir, route, &error))
    {
        std::cerr << "Failed to load route objects: " << error << std::endl;
        return 1;
    }

    std::cout << "Route: " << route.models.size() << " models, "
              << route.instances.size() << " instances" << std::endl;

    // Существующий конфиг (ручные правки сохраняем)
    const std::string config_path = route_dir + "/colliders.conf";

    collision::ColliderConfig existing;
    const bool has_existing = collision::loadColliderConfig(config_path, existing, nullptr);
    if (has_existing)
    {
        std::cout << "Existing colliders.conf: " << existing.entries.size()
                  << " entries" << std::endl;
    }

    std::map<std::string, collision::ColliderEntry> merged = existing.entries;

    std::size_t generated = 0;
    std::size_t kept = 0;
    std::size_t missing_models = 0;
    std::size_t failed = 0;

    for (const auto& [label, model_rel_path] : route.models)
    {
        if (!force && merged.count(label) != 0)
        {
            ++kept;
            continue;
        }

        const std::string model_path = route_dir + model_rel_path;
        if (!std::filesystem::exists(model_path))
        {
            std::cout << "  WARNING: model not found: " << model_path << std::endl;
            ++missing_models;
            continue;
        }

        ExtractedMesh geom = extractModelGeometry(model_path);
        if (!geom.valid || !geom.bounds.valid())
        {
            std::cout << "  WARNING: no geometry in " << model_path << std::endl;
            ++failed;
            continue;
        }

        collision::ColliderEntry entry = makeEntry(label, geom);

        if (entry.type == collision::ColliderType::Mesh)
        {
            const std::string rel_path = "/models/colliders/" + label + ".colmesh";
            if (!writeColmeshFile(route_dir + rel_path, geom))
            {
                std::cout << "  WARNING: failed to write " << rel_path << std::endl;
                ++failed;
                continue;
            }
            entry.file = rel_path;
        }

        merged[label] = entry;
        ++generated;

        std::cout << "  " << label << " -> " << serializeEntry(label, entry)
                  << " (" << geom.triangles.size() << " tris)" << std::endl;
    }

    // Итоговый colliders.conf
    std::ofstream out(config_path);
    if (!out)
    {
        std::cerr << "Failed to write " << config_path << std::endl;
        return 1;
    }

    out << "# colliders.conf - сгенерирован collider-gen\n"
        << "# Повторный запуск без --force сохраняет существующие записи\n"
        << "# (ручные правки не теряются); --force перегенерирует всё.\n"
        << "# Формат: <label> <none|box hx hy hz|sphere r|capsule hh r|cylinder hh r|mesh файл>\n"
        << "#         [offset x y z] [layer <слой>] [profile <профиль>]\n";

    for (const auto& [label, entry] : merged)
        out << serializeEntry(label, entry) << '\n';

    std::cout << "Done: " << generated << " generated, " << kept << " kept, "
              << missing_models << " models missing, " << failed << " failed"
              << std::endl;

    return 0;
}
