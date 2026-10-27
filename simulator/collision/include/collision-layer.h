//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision layer utilities and body groups
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_LAYER_H
#define     COLLISION_LAYER_H

#include    "collision-export.h"
#include    "collision-types.h"

#include    <cstdint>

namespace collision
{

/// Идентификатор группы тел, между которыми контакты отключены
/// (например, свои узлы внутри одной единицы ПС). 0 - группы нет
using GroupId = std::uint64_t;
constexpr GroupId no_group = 0;

/// Человекочитаемое имя слоя (для отладки и редактора)
COLLISION_EXPORT const char* layerName(Layer layer);

/// Слой по имени (для конфигов). При неизвестном имени возвращает
/// fallback и выставляет ok = false
COLLISION_EXPORT Layer layerFromString(const char* name,
                                       bool* ok = nullptr,
                                       Layer fallback = Layer::Infrastructure);

} // namespace collision

#endif // COLLISION_LAYER_H
