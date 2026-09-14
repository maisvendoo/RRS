//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision object (body handle)
//
//------------------------------------------------------------------------------

#include    "collision-object.h"
#include    "collision-world.h"

namespace collision
{

// Значение невалидного BodyID (JPH::BodyID::cInvalidBodyID)
static constexpr std::uint32_t invalid_body_id = 0xFFFFFFFFu;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionObject::CollisionObject()
    : world_(nullptr)
    , body_id_(invalid_body_id)
    , layer_(Layer::Terrain)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionObject::CollisionObject(CollisionWorld* world,
                                 std::uint32_t body_id,
                                 Layer layer)
    : world_(world)
    , body_id_(body_id)
    , layer_(layer)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionObject::isValid() const
{
    return world_ != nullptr && body_id_ != invalid_body_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::uint32_t CollisionObject::id() const
{
    return body_id_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Layer CollisionObject::layer() const
{
    return layer_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void* CollisionObject::userData() const
{
    if (!isValid())
        return nullptr;

    return world_->objectUserData(body_id_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionObject::setUserData(void* user_data)
{
    if (!isValid())
        return;

    world_->setObjectUserData(body_id_, user_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Vec3f CollisionObject::position() const
{
    if (!isValid())
        return Vec3f();

    return world_->objectPosition(body_id_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Quatf CollisionObject::rotation() const
{
    if (!isValid())
        return Quatf();

    return world_->objectRotation(body_id_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionObject::setPositionRotation(const Vec3f& position,
                                          const Quatf& rotation)
{
    if (!isValid())
        return;

    world_->setObjectPositionRotation(body_id_, position, rotation);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CollisionObject::setActive(bool active)
{
    if (!isValid())
        return;

    world_->setObjectActive(body_id_, active);
}

} // namespace collision
