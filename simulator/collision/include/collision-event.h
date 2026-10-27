//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision events and thread-safe event queue
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_EVENT_H
#define     COLLISION_EVENT_H

#include    "collision-export.h"
#include    "collision-types.h"

#include    <cstddef>
#include    <cstdint>
#include    <deque>
#include    <mutex>

namespace collision
{

/// Тип события коллизий
enum class EventType : std::uint8_t
{
    ContactAdded,       ///< Новый контакт
    ContactPersisted,   ///< Контакт продолжается
    ContactRemoved,     ///< Контакт разорван (точка/нормаль недоступны)
    BodyActivated,      ///< Тело проснулось
    BodyDeactivated     ///< Тело заснуло
};

/// Событие коллизий
struct CollisionEvent
{
    EventType type = EventType::ContactAdded;

    std::uint32_t body_a = 0;   ///< Идентификаторы тел (упакованные BodyID)
    std::uint32_t body_b = 0;

    void* user_data_a = nullptr;    ///< user_data из ObjectDesc участников
    void* user_data_b = nullptr;

    /// Слои участников (Layer::Count - неизвестен;
    /// для ContactRemoved тела недоступны, слои всегда Layer::Count)
    Layer layer_a = Layer::Count;
    Layer layer_b = Layer::Count;

    Vec3f point;                ///< Точка контакта (мировые координаты), м
    Vec3f normal;               ///< Нормаль контакта (мировые координаты)
    float penetration = 0.0f;   ///< Глубина проникновения, м

    /// Оценка нормального импульса контакта, Н*с (ТЗ "Коллизии" п.8, P1-5).
    /// Слушатель вызывается до солвера, поэтому импульс оценивается
    /// штатной JPH::EstimateCollisionResponse, а для пар кинематик/
    /// статик (бесконечная масса) - по приведённой массе и скорости
    /// сближения (скорость кинематических тел известна Jolt'у)
    double impulse = 0.0;

    /// Скорость сближения по нормали, м/с (положительная - сближение)
    double relative_velocity = 0.0;
};

//------------------------------------------------------------------------------
/// Потокобезопасная очередь событий: слушатели Jolt вызываются из рабочих
/// потоков физики, а игра забирает события из своего потока после step().
/// Очередь ограничена сверху (high-water mark): если потребитель не забирает
/// события, при переполнении вытесняются самые старые
//------------------------------------------------------------------------------
class COLLISION_EXPORT CollisionEventQueue
{
public:

    /// Добавить событие (из потоков Jolt)
    void push(const CollisionEvent& event);

    /// Забрать следующее событие; false - очередь пуста
    bool poll(CollisionEvent& event);

    /// Очистить очередь
    void clear();

    /// Число событий в очереди
    std::size_t size() const;

private:

    /// Верхняя граница очереди: при push сверх неё отбрасываются
    /// самые старые события, память не растёт неограниченно
    static constexpr std::size_t kMaxEvents = 10000;

    mutable std::mutex mutex_;
    std::deque<CollisionEvent> events_;
};

} // namespace collision

#endif // COLLISION_EVENT_H
