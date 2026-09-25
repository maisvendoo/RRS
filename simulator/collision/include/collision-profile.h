//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Contact material profiles (friction, restitution, damping)
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_PROFILE_H
#define     COLLISION_PROFILE_H

#include    "collision-export.h"

namespace collision
{

/// Профиль контактного материала тела
struct CollisionProfile
{
    float friction        = 0.5f;   ///< Трение (обычно 0..1)
    float restitution     = 0.0f;   ///< Упругость (0 - неупругий удар)
    float linear_damping  = 0.05f;  ///< Линейное затухание dv/dt = -c*v
    float angular_damping = 0.05f;  ///< Угловое затухание dw/dt = -c*w
    bool  is_sensor       = false;  ///< Сенсор: события есть, отклика нет
};

/// Профиль по умолчанию
COLLISION_EXPORT CollisionProfile profileDefault();

/// Рельс: сталь, катание колеса
COLLISION_EXPORT CollisionProfile profileRail();

/// Колесо: сталь по стали
COLLISION_EXPORT CollisionProfile profileWheel();

/// Рельеф/земля
COLLISION_EXPORT CollisionProfile profileTerrain();

/// Бетон (платформы, стены)
COLLISION_EXPORT CollisionProfile profileConcrete();

/// Кузов подвижного состава
COLLISION_EXPORT CollisionProfile profileVehicleBody();

/// Растительность: сенсор без физического отклика
COLLISION_EXPORT CollisionProfile profileVegetation();

/// Профиль по имени ("default", "rail", "wheel", "terrain", "concrete",
/// "vehicle_body", "vegetation"). При неизвестном имени - profileDefault()
/// и ok = false
COLLISION_EXPORT CollisionProfile profileFromString(const char* name,
                                                    bool* ok = nullptr);

} // namespace collision

#endif // COLLISION_PROFILE_H
