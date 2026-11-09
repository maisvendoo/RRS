//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Route files parser (objects.ref, topology/map/route1.map)
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_ROUTE_H
#define     COLLISION_ROUTE_H

#include    "collision-export.h"
#include    "collision-types.h"

#include    <cstddef>
#include    <map>
#include    <string>
#include    <vector>

namespace collision
{

/// Экземпляр объекта маршрута из route1.map
struct RouteObjectInstance
{
    std::string label;      ///< Метка модели из objects.ref
    Vec3f       position;   ///< Мировое положение, м
    Vec3f       euler_deg;  ///< Углы поворота как в файле, градусы
};

/// Данные о статических объектах маршрута
struct RouteObjectsData
{
    std::map<std::string, std::string> models;  ///< label -> путь к модели
    std::vector<RouteObjectInstance> instances; ///< Расстановка объектов
    std::size_t skipped_lines = 0;              ///< Битые строки
};

/// Прочитать objects.ref и topology/map/route1.map из каталога маршрута.
/// Пути жёстко заданы - как в работающем коде вьювера (RouteLoader).
/// Отсутствие одного из файлов - не ошибка (маршрут может быть пустым),
/// ошибка только при нечитаемом существующем файле
COLLISION_EXPORT bool loadRouteObjects(const std::string& route_dir,
                                       RouteObjectsData& out,
                                       std::string* error = nullptr);

/// Перевод углов route1.map в кватернион мирового поворота.
/// Повторяет вьювер (RouteViewer): M = T * Rz * Ry * Rx,
/// углы берутся с инверсией знака (наследие ZDSimulator)
COLLISION_EXPORT Quatf quatFromRouteEuler(const Vec3f& euler_deg);

} // namespace collision

#endif // COLLISION_ROUTE_H
