//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision shapes (box, sphere, capsule, hull, mesh, heightfield)
//
//------------------------------------------------------------------------------

#include    "collision-shape-impl.h"

#include    <Jolt/Jolt.h>

#include    <Jolt/Math/Float3.h>
#include    <Jolt/Geometry/IndexedTriangle.h>
#include    <Jolt/Physics/Collision/Shape/BoxShape.h>
#include    <Jolt/Physics/Collision/Shape/SphereShape.h>
#include    <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include    <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include    <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include    <Jolt/Physics/Collision/Shape/MeshShape.h>
#include    <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

#include    <cstdarg>
#include    <cstdio>
#include    <cstring>
#include    <fstream>

namespace collision
{

namespace
{

//------------------------------------------------------------------------------
/// Диагностика ошибок создания/загрузки форм. libJournal завязан на Qt
/// (QString/QHash/QMutex), а модуль коллизий по построению - чистый C++
/// без Qt (см. CMakeLists), поэтому сообщение уходит в stderr - туда же,
/// куда его выводит консольное хранилище журнала. Отказ виден вызывающему
/// и по возвращаемой пустой форме
//------------------------------------------------------------------------------
void logError(const char* format, ...)
{
    std::va_list args;
    va_start(args, format);

    std::fprintf(stderr, "collision: ");
    std::vfprintf(stderr, format, args);
    std::fprintf(stderr, "\n");

    va_end(args);
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape::CollisionShape() = default;

CollisionShape::~CollisionShape() = default;

CollisionShape::CollisionShape(const CollisionShape&) = default;
CollisionShape& CollisionShape::operator=(const CollisionShape&) = default;
CollisionShape::CollisionShape(CollisionShape&&) noexcept = default;
CollisionShape& CollisionShape::operator=(CollisionShape&&) noexcept = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <typename SettingsT>
CollisionShape CollisionShape::createFrom(SettingsT& settings)
{
    // Settings живут на стеке - помечаем embedded, чтобы ref-counting
    // не пытался их удалить
    settings.SetEmbedded();

    JPH::ShapeSettings::ShapeResult result = settings.Create();

    if (result.HasError())
    {
        logError("shape creation failed: %s", result.GetError().c_str());
        return CollisionShape();
    }

    CollisionShape shape;
    shape.impl_ = std::make_shared<Impl>();
    shape.impl_->shape = result.Get();
    return shape;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::box(const Vec3f& halfExtents)
{
    JPH::BoxShapeSettings settings(
        JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::sphere(float radius)
{
    JPH::SphereShapeSettings settings(radius);
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::capsule(float halfHeight, float radius)
{
    JPH::CapsuleShapeSettings settings(halfHeight, radius);
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::cylinder(float halfHeight, float radius)
{
    JPH::CylinderShapeSettings settings(halfHeight, radius);
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::convexHull(const std::vector<Vec3f>& points)
{
    if (points.size() < 4)
        return CollisionShape();

    std::vector<JPH::Vec3> jolt_points;
    jolt_points.reserve(points.size());
    for (const Vec3f& p : points)
        jolt_points.emplace_back(p.x, p.y, p.z);

    JPH::ConvexHullShapeSettings settings(jolt_points.data(),
                                          static_cast<int>(jolt_points.size()));
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::mesh(const std::vector<Vec3f>& vertices,
                                    const std::vector<Triangle>& triangles)
{
    if (vertices.empty() || triangles.empty())
        return CollisionShape();

    JPH::VertexList jolt_vertices;
    jolt_vertices.reserve(vertices.size());
    for (const Vec3f& v : vertices)
        jolt_vertices.emplace_back(v.x, v.y, v.z);

    JPH::IndexedTriangleList jolt_triangles;
    jolt_triangles.reserve(triangles.size());
    for (const Triangle& t : triangles)
        jolt_triangles.emplace_back(t.i0, t.i1, t.i2, 0);

    JPH::MeshShapeSettings settings(jolt_vertices, jolt_triangles);
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::meshFromFile(const char* path)
{
    // Счётчики читаются из файла и определяют размер аллокаций,
    // поэтому ограничиваем их сверху: повреждённый файл не должен
    // уводить память в разнос (2 млн треугольников = 6 млн индексов)
    constexpr std::uint32_t max_vertices = 1'000'000;
    constexpr std::uint32_t max_triangles = 2'000'000;

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        logError("colmesh '%s' is not opened", path);
        return CollisionShape();
    }

    char magic[4] = {};
    file.read(magic, 4);
    if (!file || std::memcmp(magic, "RCM1", 4) != 0)
    {
        logError("colmesh '%s' has invalid magic", path);
        return CollisionShape();
    }

    std::uint32_t vertex_count = 0;
    std::uint32_t triangle_count = 0;
    file.read(reinterpret_cast<char*>(&vertex_count), sizeof(vertex_count));
    file.read(reinterpret_cast<char*>(&triangle_count), sizeof(triangle_count));
    if (!file || vertex_count < 3 || triangle_count == 0)
    {
        logError("colmesh '%s' has invalid counts: %u vertices, %u triangles",
                 path, vertex_count, triangle_count);
        return CollisionShape();
    }

    if (vertex_count > max_vertices || triangle_count > max_triangles)
    {
        logError("colmesh '%s' rejected: %u vertices / %u triangles exceed limits %u / %u",
                 path, vertex_count, triangle_count, max_vertices, max_triangles);
        return CollisionShape();
    }

    std::vector<Vec3f> vertices(vertex_count);
    std::vector<Triangle> triangles(triangle_count);
    file.read(reinterpret_cast<char*>(vertices.data()),
              vertex_count * sizeof(Vec3f));
    file.read(reinterpret_cast<char*>(triangles.data()),
              triangle_count * sizeof(Triangle));
    if (!file)
    {
        logError("colmesh '%s' is truncated", path);
        return CollisionShape();
    }

    // Индексы треугольников обязаны ссылаться на прочитанные вершины
    for (const Triangle& t : triangles)
    {
        if (t.i0 >= vertex_count || t.i1 >= vertex_count || t.i2 >= vertex_count)
        {
            logError("colmesh '%s' rejected: triangle index out of range (%u, %u, %u; vertices %u)",
                     path, t.i0, t.i1, t.i2, vertex_count);
            return CollisionShape();
        }
    }

    return mesh(vertices, triangles);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CollisionShape CollisionShape::heightField(const float* samples,
                                           int sample_count,
                                           const Vec3f& offset,
                                           const Vec3f& scale)
{
    if (samples == nullptr || sample_count < 3)
        return CollisionShape();

    JPH::HeightFieldShapeSettings settings(
        samples,
        JPH::Vec3(offset.x, offset.y, offset.z),
        JPH::Vec3(scale.x, scale.y, scale.z),
        static_cast<JPH::uint32>(sample_count));
    return createFrom(settings);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CollisionShape::isValid() const
{
    return impl_ != nullptr && impl_->shape != nullptr;
}

} // namespace collision
