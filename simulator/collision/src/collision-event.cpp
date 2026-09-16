//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision events and thread-safe event queue
//
//------------------------------------------------------------------------------

#include    "collision-event.h"

namespace collision
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionEventQueue::push(const CollisionEvent& event)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // High-water mark: если потребитель не опрашивает очередь (или не
    // успевает), вытесняем самые старые события - память не растёт
    if (events_.size() >= kMaxEvents)
        events_.pop_front();

    events_.push_back(event);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionEventQueue::poll(CollisionEvent& event)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (events_.empty())
        return false;

    event = events_.front();
    events_.pop_front();
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionEventQueue::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    events_.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::size_t CollisionEventQueue::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return events_.size();
}

} // namespace collision
