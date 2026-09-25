//------------------------------------------------------------------------------
//
//      Vehicle collision bodies (colliders over Jolt Physics)
//
//------------------------------------------------------------------------------

#include    "vehicle-collision.h"

#include    "profile-point.h"

#include    <collision-profile.h>
#include    <collision-shape.h>
#include    <collision-world.h>

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

namespace
{

//------------------------------------------------------------------------------
/// Кватернион из матрицы поворота (столбцы - образы локальных осей)
//------------------------------------------------------------------------------
collision::Quatf quatFromBasis(double m00, double m01, double m02,
                               double m10, double m11, double m12,
                               double m20, double m21, double m22)
{
    collision::Quatf q;

    const double trace = m00 + m11 + m22;
    if (trace > 0.0)
    {
        const double s = std::sqrt(trace + 1.0) * 2.0;  // s = 4w
        q.w = static_cast<float>(0.25 * s);
        q.x = static_cast<float>((m21 - m12) / s);
        q.y = static_cast<float>((m02 - m20) / s);
        q.z = static_cast<float>((m10 - m01) / s);
    }
    else if (m00 > m11 && m00 > m22)
    {
        const double s = std::sqrt(1.0 + m00 - m11 - m22) * 2.0;  // s = 4x
        q.w = static_cast<float>((m21 - m12) / s);
        q.x = static_cast<float>(0.25 * s);
        q.y = static_cast<float>((m01 + m10) / s);
        q.z = static_cast<float>((m02 + m20) / s);
    }
    else if (m11 > m22)
    {
        const double s = std::sqrt(1.0 + m11 - m00 - m22) * 2.0;  // s = 4y
        q.w = static_cast<float>((m02 - m20) / s);
        q.x = static_cast<float>((m01 + m10) / s);
        q.y = static_cast<float>(0.25 * s);
        q.z = static_cast<float>((m12 + m21) / s);
    }
    else
    {
        const double s = std::sqrt(1.0 + m22 - m00 - m11) * 2.0;  // s = 4z
        q.w = static_cast<float>((m10 - m01) / s);
        q.x = static_cast<float>((m02 + m20) / s);
        q.y = static_cast<float>((m12 + m21) / s);
        q.z = static_cast<float>(0.25 * s);
    }

    // Нормализация на случай погрешностей ортонормированности базиса
    const float n = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (n > 1e-6f)
    {
        q.x /= n;
        q.y /= n;
        q.z /= n;
        q.w /= n;
    }

    return q;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::loadConfig(QString cfg_path, double length,
                                  std::size_t num_axis_, double wheel_diameter)
{
    // Умолчания из параметров ПЕ
    body_half_extents.x = static_cast<float>(length * 0.5);
    bogie_offset = static_cast<float>(std::max(length * 0.5 - 2.1, 1.0));
    wheel_radius = static_cast<float>(wheel_diameter * 0.5);
    num_axis = num_axis_;

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString secName = "Collision";

    cfg.getBool(secName, "Enabled", enabled);

    double value = 0.0;
    if (cfg.getDouble(secName, "BodyHalfLength", value))
        body_half_extents.x = static_cast<float>(value);
    if (cfg.getDouble(secName, "BodyHalfWidth", value))
        body_half_extents.y = static_cast<float>(value);
    if (cfg.getDouble(secName, "BodyHalfHeight", value))
        body_half_extents.z = static_cast<float>(value);
    if (cfg.getDouble(secName, "BodyOffsetZ", value))
        body_offset_z = static_cast<float>(value);

    cfg.getInt(secName, "NumBogies", num_bogies);

    if (cfg.getDouble(secName, "BogieHalfLength", value))
        bogie_half_extents.x = static_cast<float>(value);
    if (cfg.getDouble(secName, "BogieHalfWidth", value))
        bogie_half_extents.y = static_cast<float>(value);
    if (cfg.getDouble(secName, "BogieHalfHeight", value))
        bogie_half_extents.z = static_cast<float>(value);
    if (cfg.getDouble(secName, "BogieOffset", value))
        bogie_offset = static_cast<float>(value);
    if (cfg.getDouble(secName, "BogieOffsetZ", value))
        bogie_offset_z = static_cast<float>(value);

    if (cfg.getDouble(secName, "WheelsetSpacing", value))
        wheelset_spacing = static_cast<float>(value);
    if (cfg.getDouble(secName, "WheelsetHalfWidth", value))
        wheelset_half_width = static_cast<float>(value);
    if (cfg.getDouble(secName, "WheelRadius", value))
        wheel_radius = static_cast<float>(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleCollision::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::addPart(collision::CollisionWorld& world,
                               const collision::CollisionShape& shape,
                               collision::Layer layer,
                               const collision::CollisionProfile& profile,
                               const collision::Vec3f& local_offset,
                               collision::GroupId group,
                               void* user_data,
                               PartKind kind)
{
    if (!shape.isValid())
        return;

    collision::ObjectDesc desc;
    desc.shape = shape;
    desc.layer = layer;
    desc.motion = collision::MotionType::Kinematic;
    desc.profile = profile;
    desc.group = group;
    desc.user_data = user_data;
    // Кинематика должна видеть статический мир (события контактов)
    desc.collide_kinematic_vs_static = true;
    // CCD (ТЗ "Коллизии" п.24, P2-12): линейный каст исключает
    // туннелирование сквозь тонкие препятствия на скорости
    desc.motion_quality = collision::MotionQuality::LinearCast;

    Part part;
    part.object = world.createObject(desc);
    part.local_offset = local_offset;

    // Данные создания - для пересоздания части после возврата с L2
    part.shape = shape;
    part.layer = layer;
    part.profile = profile;
    part.group = group;
    part.user_data = user_data;
    part.kind = kind;

    if (part.object.isValid())
        parts.push_back(part);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::recreatePart(collision::CollisionWorld& world,
                                    Part& part)
{
    collision::ObjectDesc desc;
    desc.shape = part.shape;
    desc.layer = part.layer;
    desc.motion = collision::MotionType::Kinematic;
    desc.motion_quality = collision::MotionQuality::LinearCast;
    desc.profile = part.profile;
    desc.group = part.group;
    desc.user_data = part.user_data;
    desc.collide_kinematic_vs_static = true;
    desc.position = part.last_position;
    desc.rotation = part.last_rotation;

    part.object = world.createObject(desc);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::createBodies(collision::CollisionWorld& world,
                                    collision::GroupId group,
                                    void* user_data)
{
    if (!enabled || !world.isInitialized() || hasBodies())
        return;

    // Дальняя ПС (LOD >= L2): только box кузова (ТЗ "Коллизии" п.27)
    const bool detailed = (lod < perf::SimLOD::L2_Aggregated);

    // Кузов
    addPart(world,
            collision::CollisionShape::box(body_half_extents),
            collision::Layer::Train,
            collision::profileVehicleBody(),
            collision::Vec3f(0.0f, 0.0f, body_offset_z),
            group, user_data, PartKind::Body);

    // Тележки
    if (detailed && num_bogies > 0)
    {
        const collision::CollisionShape bogie_shape =
            collision::CollisionShape::box(bogie_half_extents);

        for (int i = 0; i < num_bogies; ++i)
        {
            float bogie_x = 0.0f;
            if (num_bogies > 1)
            {
                bogie_x = -bogie_offset +
                    2.0f * bogie_offset * static_cast<float>(i) /
                    static_cast<float>(num_bogies - 1);
            }

            addPart(world, bogie_shape,
                    collision::Layer::Bogie,
                    collision::profileVehicleBody(),
                    collision::Vec3f(bogie_x, 0.0f, bogie_offset_z),
                    group, user_data, PartKind::Bogie);
        }

        // Колёсные пары (ось цилиндра Jolt - Y, совпадает с поперечной осью)
        if (num_axis > 0 && wheel_radius > 0.0f)
        {
            const collision::CollisionShape wheelset_shape =
                collision::CollisionShape::cylinder(wheelset_half_width,
                                                    wheel_radius);

            std::size_t axle_idx = 0;
            for (int i = 0; i < num_bogies && axle_idx < num_axis; ++i)
            {
                float bogie_x = 0.0f;
                if (num_bogies > 1)
                {
                    bogie_x = -bogie_offset +
                        2.0f * bogie_offset * static_cast<float>(i) /
                        static_cast<float>(num_bogies - 1);
                }

                // Оси внутри тележки (остаток - в первые тележки)
                const std::size_t axles_here =
                    num_axis / static_cast<std::size_t>(num_bogies) +
                    (static_cast<std::size_t>(i) <
                     num_axis % static_cast<std::size_t>(num_bogies) ? 1 : 0);

                for (std::size_t j = 0; j < axles_here && axle_idx < num_axis; ++j, ++axle_idx)
                {
                    const float axle_x = bogie_x + wheelset_spacing *
                        (static_cast<float>(j) - 0.5f * static_cast<float>(axles_here - 1));

                    addPart(world, wheelset_shape,
                            collision::Layer::Wheel,
                            collision::profileWheel(),
                            collision::Vec3f(axle_x, 0.0f, wheel_radius),
                            group, user_data, PartKind::Wheel);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::syncPose(const profile_point_t& profile_point)
{
    if (parts.empty())
        return;

    const dvec3& o = profile_point.orth;    // вдоль пути
    const dvec3& r = profile_point.right;   // вправо
    const dvec3& u = profile_point.up;      // вверх

    // Тройка (orth, right, up) в RRS - левосторонняя, для кватерниона
    // нужна правосторонняя: локальная ось Y тела - ВЛЕВО (-right).
    // На симметрию коллайдеров (box/cylinder) это не влияет
    const dvec3 left(-r.x, -r.y, -r.z);

    const collision::Quatf rotation = quatFromBasis(
        o.x, left.x, u.x,
        o.y, left.y, u.y,
        o.z, left.z, u.z);

    for (Part& part : parts)
    {
        const dvec3 w = profile_point.position +
            o * static_cast<double>(part.local_offset.x) +
            r * static_cast<double>(part.local_offset.y) +
            u * static_cast<double>(part.local_offset.z);

        // Кэш последней позы: пересозданные после LOD-упрощения
        // тела сразу ставятся в актуальное положение
        part.last_position = collision::Vec3f(
                static_cast<float>(w.x),
                static_cast<float>(w.y),
                static_cast<float>(w.z));
        part.last_rotation = rotation;

        // Часть может быть временно убрана (LOD >= L2)
        if (!part.object.isValid())
            continue;

        part.object.setPositionRotation(part.last_position, rotation);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::removeBodies(collision::CollisionWorld& world)
{
    for (Part& part : parts)
        world.removeObject(part.object);

    parts.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleCollision::setLod(perf::SimLOD new_lod,
                              collision::CollisionWorld& world)
{
    if (new_lod == lod)
        return;

    lod = new_lod;

    // Тела ещё не созданы: уровень применится при createBodies()
    if (!hasBodies())
        return;

    // Дальняя ПС - только кузов; близкая - полный набор
    // (ТЗ "Коллизии" п.27)
    const bool detailed = (lod < perf::SimLOD::L2_Aggregated);

    for (Part& part : parts)
    {
        if (part.kind == PartKind::Body)
            continue;

        if (!detailed && part.object.isValid())
        {
            // Уход на L2+: убрать коллайдеры тележек/колёс из мира
            world.removeObject(part.object);
        }
        else if (detailed && !part.object.isValid() && part.shape.isValid())
        {
            // Возврат на L0/L1: пересоздать тела на последней позе ПС
            recreatePart(world, part);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
perf::SimLOD VehicleCollision::getLod() const
{
    return lod;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleCollision::hasBodies() const
{
    return !parts.empty();
}
