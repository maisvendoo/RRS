//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision shapes (box, sphere, capsule, hull, mesh, heightfield)
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_SHAPE_H
#define     COLLISION_SHAPE_H

#include    "collision-export.h"
#include    "collision-types.h"

#include    <cstdint>
#include    <memory>
#include    <vector>

namespace collision
{

/// Треугольник для mesh-формы (индексы вершин)
struct Triangle
{
    std::uint32_t i0 = 0;
    std::uint32_t i1 = 0;
    std::uint32_t i2 = 0;
};

//------------------------------------------------------------------------------
/// Коллизионная форма. Разделяемый immutable-ресурс: копии дёшевы
/// (внутри ref-counted), одну форму можно переиспользовать для многих тел
//------------------------------------------------------------------------------
class COLLISION_EXPORT CollisionShape
{
public:

    CollisionShape();
    ~CollisionShape();

    CollisionShape(const CollisionShape&);
    CollisionShape& operator=(const CollisionShape&);
    CollisionShape(CollisionShape&&) noexcept;
    CollisionShape& operator=(CollisionShape&&) noexcept;

    /// Параллелепипед: полуразмеры, м
    static CollisionShape box(const Vec3f& halfExtents);

    /// Сфера: радиус, м
    static CollisionShape sphere(float radius);

    /// Капсула (ось Y): половина высоты цилиндрической части и радиус, м
    static CollisionShape capsule(float halfHeight, float radius);

    /// Цилиндр (ось Y): половина высоты и радиус, м
    static CollisionShape cylinder(float halfHeight, float radius);

    /// Выпуклая оболочка по облаку точек, м
    static CollisionShape convexHull(const std::vector<Vec3f>& points);

    /// Треугольная сетка (ТОЛЬКО для статических тел), м
    static CollisionShape mesh(const std::vector<Vec3f>& vertices,
                               const std::vector<Triangle>& triangles);

    /// Треугольная сетка из бинарного .colmesh-файла (пишет Collision Editor).
    /// Формат: magic "RCM1" (4 байта), uint32 vertex_count, uint32
    /// triangle_count, далее vertex_count Vec3f и triangle_count Triangle
    static CollisionShape meshFromFile(const char* path);

    /// Рельеф: сетка высот sample_count x sample_count, записанная построчно.
    /// Поверхность: offset + scale * (x, samples[y * sample_count + x], y),
    /// где x, y - целые в диапазоне [0, sample_count - 1]
    static CollisionShape heightField(const float* samples,
                                      int sample_count,
                                      const Vec3f& offset,
                                      const Vec3f& scale);

    /// Форма создана успешно
    bool isValid() const;

private:

    struct Impl;
    std::shared_ptr<Impl> impl_;

    template <typename SettingsT>
    static CollisionShape createFrom(SettingsT& settings);

    friend class CollisionWorld;
};

} // namespace collision

#endif // COLLISION_SHAPE_H
