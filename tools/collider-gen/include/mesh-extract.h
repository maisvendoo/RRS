//------------------------------------------------------------------------------
//
//      collider-gen: collision data generator for RRS routes
//      Geometry extraction from vsg scene graphs
//
//------------------------------------------------------------------------------

#ifndef     MESH_EXTRACT_H
#define     MESH_EXTRACT_H

#include    <collision-types.h>
#include    <collision-shape.h>

#include    <vsg/core/ref_ptr.h>
#include    <vsg/maths/box.h>

#include    <string>
#include    <vector>

namespace vsg
{
    class Object;
}

/// Извлечённая из модели геометрия (локальные координаты модели, метры)
struct ExtractedMesh
{
    std::vector<collision::Vec3f>    vertices;
    std::vector<collision::Triangle> triangles;
    vsg::dbox bounds;           ///< AABB (bounds.valid() == false, если пусто)

    bool valid = false;
};

/// Загрузить модель (gltf через vsgXchange) и извлечь треугольники + AABB
ExtractedMesh extractModelGeometry(const std::string& filename);

/// Записать .colmesh (бинарный формат CollisionShape::meshFromFile)
bool writeColmeshFile(const std::string& path, const ExtractedMesh& mesh);

#endif // MESH_EXTRACT_H
