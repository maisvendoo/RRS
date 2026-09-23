//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision world wrapper over Jolt PhysicsSystem
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_WORLD_H
#define     COLLISION_WORLD_H

#include    "collision-export.h"
#include    "collision-types.h"
#include    "collision-event.h"
#include    "collision-object.h"

#include    <cstdint>
#include    <memory>

namespace JPH
{
    class PhysicsSystem;
}

namespace collision
{

/// Настройки мира коллизий
struct WorldSettings
{
    unsigned int max_bodies               = 65536;          ///< Макс. число тел
    unsigned int num_body_mutexes         = 0;              ///< 0 - по умолчанию Jolt
    unsigned int max_body_pairs           = 65536;          ///< Макс. число пар тел
    unsigned int max_contact_constraints  = 10240;          ///< Макс. число контактов
    unsigned int temp_allocator_size      = 16 * 1024 * 1024; ///< Временный аллокатор, байт
    int          worker_threads           = -1;             ///< -1 - все ядра минус одно

    /// Условная масса кинематического тела ПС, кг: для оценки импульса
    /// контакта пар "кинематик/статик" (обратные массы бесконечны,
    /// солвер дал бы 0). ~60 т секции тепловоза (ТЗ "Коллизии" п.8)
    float        kinematic_impact_mass    = 60000.0f;
};

//------------------------------------------------------------------------------
/// Физический мир коллизий (обёртка над Jolt PhysicsSystem)
//------------------------------------------------------------------------------
class COLLISION_EXPORT CollisionWorld
{
public:

    CollisionWorld();
    ~CollisionWorld();

    CollisionWorld(const CollisionWorld&) = delete;
    CollisionWorld& operator=(const CollisionWorld&) = delete;

    /// Инициализация Jolt и создание PhysicsSystem
    bool init(const WorldSettings& settings = WorldSettings());

    /// Освобождение всех ресурсов. Все хэндлы объектов инвалидируются
    void shutdown();

    /// Мир инициализирован
    bool isInitialized() const;

    /// Шаг симуляции коллизий.
    /// ВНИМАНИЕ: создание/удаление объектов разрешено только между
    /// вызовами step(), но не из потоков Jolt
    /// @param dt шаг интегрирования, с
    /// @param collision_steps число подшагов за вызов
    void step(float dt, int collision_steps = 1);

    /// Установить вектор гравитации, м/с^2 (по умолчанию Jolt: (0, -9.81, 0))
    void setGravity(float x, float y, float z);

    /// Создать тело и добавить в мир. При ошибке хэндл невалиден
    CollisionObject createObject(const ObjectDesc& desc);

    /// Убрать и уничтожить тело; хэндл инвалидируется
    void removeObject(CollisionObject& object);

    /// Забрать следующее событие коллизий (false - событий нет)
    bool pollEvent(CollisionEvent& event);

    /// Очистить очередь событий
    void clearEvents();

    /// Оптимизировать broadphase после массовой загрузки объектов.
    /// Дорогая операция - вызывать один раз после загрузки участка мира
    void optimizeBroadPhase();

    /// Доступ к низкоуровневой системе (для отладки и расширений)
    JPH::PhysicsSystem* physicsSystem();

    /// Луч от точки position в направлении direction (единичный),
    /// макс. max_dist. true - попадание: point - точка контакта,
    /// normal - нормаль поверхности. Для пешего режима/прицеливания
    bool raycast(const Vec3f& position,
                 const Vec3f& direction,
                 float max_dist,
                 Vec3f& point,
                 Vec3f& normal);

    /// Могут ли два слоя сталкиваться (матрица контактов)
    static bool canLayersCollide(Layer a, Layer b);

private:

    friend class CollisionObject;

    Vec3f objectPosition(std::uint32_t body_id) const;
    Quatf objectRotation(std::uint32_t body_id) const;
    void  setObjectPositionRotation(std::uint32_t body_id,
                                    const Vec3f& position,
                                    const Quatf& rotation);
    void  setObjectActive(std::uint32_t body_id, bool active);
    void* objectUserData(std::uint32_t body_id) const;
    void  setObjectUserData(std::uint32_t body_id, void* user_data);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace collision

#endif // COLLISION_WORLD_H
