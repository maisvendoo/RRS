//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      World loader: route objects + colliders.conf -> CollisionWorld
//
//------------------------------------------------------------------------------

#include    "collision-world-loader.h"

#include    "collision-collider-config.h"
#include    "collision-profile.h"
#include    "collision-route.h"
#include    "collision-world.h"

#include    <algorithm>
#include    <map>

namespace collision
{

namespace
{

/// Построить форму по записи colliders.conf (кэшируется на метку)
CollisionShape buildShape(const ColliderEntry& entry,
                          const std::string& route_dir)
{
    switch (entry.type)
    {
    case ColliderType::Box:
        return CollisionShape::box(entry.half_extents);

    case ColliderType::Sphere:
        return CollisionShape::sphere(entry.radius);

    case ColliderType::Capsule:
        return CollisionShape::capsule(entry.half_height, entry.radius);

    case ColliderType::Cylinder:
        return CollisionShape::cylinder(entry.half_height, entry.radius);

    case ColliderType::Mesh:
    {
        const std::string path = route_dir + entry.file;
        return CollisionShape::meshFromFile(path.c_str());
    }

    case ColliderType::None:
    default:
        return CollisionShape();
    }
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool loadRouteIntoWorld(CollisionWorld& world,
                        const std::string& route_dir,
                        WorldLoadStats& stats,
                        std::string* error)
{
    stats = WorldLoadStats();

    RouteObjectsData route;
    if (!loadRouteObjects(route_dir, route, error))
        return false;

    // colliders.conf может отсутствовать - тогда все метки попадут в отчёт
    ColliderConfig colliders;
    const std::string colliders_path = route_dir + "/colliders.conf";
    loadColliderConfig(colliders_path, colliders, nullptr);

    stats.instances_total = route.instances.size();

    // Кэш форм: одна форма на все экземпляры модели
    std::map<std::string, CollisionShape> shape_cache;

    for (const RouteObjectInstance& instance : route.instances)
    {
        const auto entry_it = colliders.entries.find(instance.label);
        if (entry_it == colliders.entries.end())
        {
            // Метка без коллайдера - фиксируем один раз
            if (std::find(stats.labels_without_collider.begin(),
                          stats.labels_without_collider.end(),
                          instance.label) == stats.labels_without_collider.end())
            {
                stats.labels_without_collider.push_back(instance.label);
            }
            continue;
        }

        const ColliderEntry& entry = entry_it->second;
        if (entry.type == ColliderType::None)
        {
            ++stats.instances_none;
            continue;
        }

        // Форма для метки
        auto shape_it = shape_cache.find(instance.label);
        if (shape_it == shape_cache.end())
        {
            shape_it = shape_cache.emplace(instance.label,
                                           buildShape(entry, route_dir)).first;
        }

        const CollisionShape& shape = shape_it->second;
        if (!shape.isValid())
        {
            ++stats.instances_failed;
            continue;
        }

        // Трансформ маршрута; смещение коллайдера - в локальных осях модели
        const Quatf rotation = quatFromRouteEuler(instance.euler_deg);

        ObjectDesc desc;
        desc.shape = shape;
        desc.layer = entry.layer;
        desc.motion = MotionType::Static;
        desc.position = instance.position + rotate(rotation, entry.offset);
        desc.rotation = rotation;
        desc.profile = profileFromString(entry.profile.c_str());

        CollisionObject object = world.createObject(desc);
        if (object.isValid())
            ++stats.objects_created;
        else
            ++stats.instances_failed;
    }

    world.optimizeBroadPhase();
    return true;
}

} // namespace collision
