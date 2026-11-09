//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Contact material profiles (friction, restitution, damping)
//
//------------------------------------------------------------------------------

#include    "collision-profile.h"

#include    <cstring>

namespace collision
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileDefault()
{
    return CollisionProfile();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileRail()
{
    CollisionProfile profile;
    profile.friction = 0.3f;        // сталь по стали, сухая
    profile.restitution = 0.0f;
    profile.linear_damping = 0.0f;
    profile.angular_damping = 0.0f;
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileWheel()
{
    CollisionProfile profile;
    profile.friction = 0.4f;        // сталь по стали, сцепление
    profile.restitution = 0.0f;
    profile.linear_damping = 0.0f;
    profile.angular_damping = 0.0f;
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileTerrain()
{
    CollisionProfile profile;
    profile.friction = 0.8f;
    profile.restitution = 0.0f;
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileConcrete()
{
    CollisionProfile profile;
    profile.friction = 0.7f;
    profile.restitution = 0.05f;
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileVehicleBody()
{
    CollisionProfile profile;
    profile.friction = 0.4f;
    profile.restitution = 0.05f;
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileVegetation()
{
    CollisionProfile profile;
    profile.friction = 0.0f;
    profile.restitution = 0.0f;
    profile.is_sensor = true;       // только события, без физического отклика
    return profile;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionProfile profileFromString(const char* name, bool* ok)
{
    if (ok != nullptr)
        *ok = true;

    if (name != nullptr)
    {
        if (std::strcmp(name, "default") == 0)      return profileDefault();
        if (std::strcmp(name, "rail") == 0)         return profileRail();
        if (std::strcmp(name, "wheel") == 0)        return profileWheel();
        if (std::strcmp(name, "terrain") == 0)      return profileTerrain();
        if (std::strcmp(name, "concrete") == 0)     return profileConcrete();
        if (std::strcmp(name, "vehicle_body") == 0) return profileVehicleBody();
        if (std::strcmp(name, "vegetation") == 0)   return profileVegetation();
    }

    if (ok != nullptr)
        *ok = false;
    return profileDefault();
}

} // namespace collision
