//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collider definitions config (colliders.conf)
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_COLLIDER_CONFIG_H
#define     COLLISION_COLLIDER_CONFIG_H

#include    "collision-export.h"
#include    "collision-types.h"
#include    "collision-layer.h"

#include    <cstddef>
#include    <map>
#include    <string>

namespace collision
{

/// Тип формы коллайдера в colliders.conf
enum class ColliderType : std::uint8_t
{
    None,       ///< Коллизий нет (мелкие декорации, провода и т.п.)
    Box,        ///< Параллелепипед: half_extents
    Sphere,     ///< Сфера: radius
    Capsule,    ///< Капсула: half_height + radius
    Cylinder,   ///< Цилиндр: half_height + radius
    Mesh        ///< Треугольная сетка из бинарного .colmesh-файла
};

/// Описание коллайдера для метки модели.
/// Все координаты - в локальном пространстве модели, метры
struct ColliderEntry
{
    ColliderType type = ColliderType::None;

    Vec3f half_extents;         ///< Box: полуразмеры
    float  radius      = 0.0f;  ///< Sphere/Capsule/Cylinder: радиус
    float  half_height = 0.0f;  ///< Capsule/Cylinder: половина высоты
    Vec3f  offset;              ///< Смещение коллайдера относительно origin
    std::string file;           ///< Mesh: путь к .colmesh относительно маршрута

    Layer       layer = Layer::Infrastructure;  ///< Слой коллизий
    std::string profile = "default";            ///< Имя контактного профиля
};

/// Набор коллайдеров маршрута: label -> описание
struct ColliderConfig
{
    std::map<std::string, ColliderEntry> entries;
    std::size_t skipped_lines = 0;  ///< Битые строки
};

/// Прочитать colliders.conf. Формат строки (поля через пробел, # - комментарий):
///   <label> none
///   <label> box <hx> <hy> <hz>
///   <label> sphere <r>
///   <label> capsule <half_height> <r>
///   <label> cylinder <half_height> <r>
///   <label> mesh <файл.colmesh>
/// Далее опционально в любом порядке:
///   offset <x> <y> <z>   layer <имя_слоя>   profile <имя_профиля>
/// Пример:
///   op1r cylinder 3.5 0.15 offset 0 0 3.5 layer infrastructure profile concrete
///   FloorL mesh /models/colliders/FloorL.colmesh layer terrain profile terrain
COLLISION_EXPORT bool loadColliderConfig(const std::string& path,
                                         ColliderConfig& out,
                                         std::string* error = nullptr);

} // namespace collision

#endif // COLLISION_COLLIDER_CONFIG_H
