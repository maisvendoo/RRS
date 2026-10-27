//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      World loader: route objects + colliders.conf -> CollisionWorld
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_WORLD_LOADER_H
#define     COLLISION_WORLD_LOADER_H

#include    "collision-export.h"

#include    <cstddef>
#include    <string>
#include    <vector>

namespace collision
{

class CollisionWorld;

/// Статистика загрузки мира
struct WorldLoadStats
{
    std::size_t instances_total = 0;    ///< Строк в route1.map
    std::size_t objects_created = 0;    ///< Создано тел
    std::size_t instances_none = 0;     ///< Пропущено: коллайдер "none"
    std::size_t instances_failed = 0;   ///< Не удалось создать тело/форму

    /// Метки без записи в colliders.conf (для отчёта и редактора)
    std::vector<std::string> labels_without_collider;
};

/// Загрузить статические объекты маршрута в мир коллизий.
/// Читает из route_dir: objects.ref, topology/map/route1.map, colliders.conf.
/// Мир должен быть инициализирован (init() уже вызван).
/// После загрузки вызывает optimizeBroadPhase().
/// false - не прочитался ни один файл маршрута
COLLISION_EXPORT bool loadRouteIntoWorld(CollisionWorld& world,
                                         const std::string& route_dir,
                                         WorldLoadStats& stats,
                                         std::string* error = nullptr);

} // namespace collision

#endif // COLLISION_WORLD_LOADER_H
