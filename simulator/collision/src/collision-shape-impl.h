//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      INTERNAL HEADER: CollisionShape implementation details.
//      Не устанавливается в SDK, только для модулей collision.
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_SHAPE_IMPL_H
#define     COLLISION_SHAPE_IMPL_H

#include    "collision-shape.h"

#include    <Jolt/Jolt.h>
#include    <Jolt/Physics/Collision/Shape/Shape.h>

struct collision::CollisionShape::Impl
{
    JPH::ShapeRefC shape;   // ref-counted форма Jolt
};

#endif // COLLISION_SHAPE_IMPL_H
