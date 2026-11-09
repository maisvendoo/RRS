//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision object (body handle)
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_OBJECT_H
#define     COLLISION_OBJECT_H

#include    "collision-export.h"
#include    "collision-types.h"
#include    "collision-layer.h"
#include    "collision-profile.h"
#include    "collision-shape.h"

#include    <cstdint>

namespace collision
{

class CollisionWorld;

/// Тип движения тела
enum class MotionType : std::uint8_t
{
    Static,     ///< Недвижимая геометрия мира
    Kinematic,  ///< Движется по скрипту, силы на него не действуют
    Dynamic     ///< Полноценная динамика
};

/// Качество обнаружения столкновений (CCD). Jolt: EMotionQuality
enum class MotionQuality : std::uint8_t
{
    Discrete,   ///< Дискретные шаги: при высокой скорости возможно
                ///  туннелирование сквозь тонкие препятствия
    LinearCast  ///< Линейный каст формы на шаге: туннелирование
                ///  исключено (дороже, для быстрых тел ПС)
};

/// Описание создаваемого объекта коллизий
struct ObjectDesc
{
    CollisionShape shape;                       ///< Форма
    Layer        layer = Layer::Infrastructure; ///< Слой
    MotionType   motion = MotionType::Static;   ///< Тип движения

    /// CCD для быстрых тел (кинематика ПС), Jolt: EMotionQuality
    MotionQuality motion_quality = MotionQuality::Discrete;

    Vec3f position;                 ///< Начальное положение (мир), м
    Quatf rotation;                 ///< Начальный поворот (мир)

    CollisionProfile profile;       ///< Контактный материал

    GroupId group     = no_group;   ///< Тела одной группы не сталкиваются
    void*   user_data = nullptr;    ///< Произвольный указатель для событий

    /// Разрешить контакты кинематического тела со статикой и другой
    /// кинематикой (для ПС, движущегося по траектории: нужны события
    /// о контактах со статическим миром)
    bool collide_kinematic_vs_static = false;
};

//------------------------------------------------------------------------------
/// Хэндл тела в мире коллизий. Лёгкий, копируемый.
/// Невалиден после removeObject() и shutdown() мира
//------------------------------------------------------------------------------
class COLLISION_EXPORT CollisionObject
{
public:

    CollisionObject();

    /// Хэндл указывает на живое тело
    bool isValid() const;

    /// Упакованный идентификатор тела
    std::uint32_t id() const;

    /// Слой, заданный при создании
    Layer layer() const;

    /// Пользовательские данные
    void* userData() const;
    void setUserData(void* user_data);

    /// Положение и поворот тела (мировые координаты)
    Vec3f position() const;
    Quatf rotation() const;

    /// Телепорт / кинематическое перемещение (активирует тело)
    void setPositionRotation(const Vec3f& position, const Quatf& rotation);

    /// Разбудить / усыпить тело
    void setActive(bool active);

private:

    friend class CollisionWorld;

    CollisionObject(CollisionWorld* world, std::uint32_t body_id, Layer layer);

    CollisionWorld* world_ = nullptr;
    std::uint32_t   body_id_ = 0;
    Layer           layer_ = Layer::Terrain;
};

} // namespace collision

#endif // COLLISION_OBJECT_H
