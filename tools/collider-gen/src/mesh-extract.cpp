//------------------------------------------------------------------------------
//
//      collider-gen: collision data generator for RRS routes
//      Geometry extraction from vsg scene graphs
//
//------------------------------------------------------------------------------

#include    "mesh-extract.h"

#include    <graphics/common.h>

#include    <vsg/all.h>

#include    <cstdint>
#include    <filesystem>
#include    <fstream>

// Формат .colmesh должен совпадать с CollisionShape::meshFromFile
static_assert(sizeof(collision::Vec3f) == 3 * sizeof(float),
              "Vec3f must be tightly packed");
static_assert(sizeof(collision::Triangle) == 3 * sizeof(std::uint32_t),
              "Triangle must be tightly packed");

namespace
{

//------------------------------------------------------------------------------
/// Собирает все треугольники графа сцены в локальных координатах модели
//------------------------------------------------------------------------------
class GeometryExtractor : public vsg::Visitor
{
public:

    explicit GeometryExtractor(ExtractedMesh& out)
        : out_(out)
    {
    }

    // Всё, что не распознано явно, просто обходим вглубь
    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::MatrixTransform& transform) override
    {
        const vsg::dmat4 saved = matrix_;
        matrix_ = matrix_ * transform.matrix;
        transform.traverse(*this);
        matrix_ = saved;
    }

    void apply(vsg::VertexIndexDraw& vid) override
    {
        const vsg::vec3Array* positions = findPositions(vid.arrays);
        if (positions == nullptr || vid.indices == nullptr)
            return;

        const std::uint32_t base = appendVertices(positions);

        std::vector<std::uint32_t> indices;
        if (!readIndices(*vid.indices, vid.firstIndex, vid.indexCount, indices))
            return;

        // Модели маршрутов - треугольные сетки (dmd2gltf триангулирует)
        for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            collision::Triangle triangle;
            triangle.i0 = base + indices[i];
            triangle.i1 = base + indices[i + 1];
            triangle.i2 = base + indices[i + 2];
            out_.triangles.push_back(triangle);
        }
    }

    void apply(vsg::VertexDraw& vd) override
    {
        const vsg::vec3Array* positions = findPositions(vd.arrays);
        if (positions == nullptr)
            return;

        const std::uint32_t base = appendVertices(positions);

        const std::size_t first = vd.firstVertex;
        const std::size_t count = (vd.vertexCount != 0)
            ? static_cast<std::size_t>(vd.vertexCount)
            : (positions->size() - first);
        const std::size_t last = std::min(first + count, positions->size());

        for (std::size_t i = first; i + 2 < last; i += 3)
        {
            collision::Triangle triangle;
            triangle.i0 = base + static_cast<std::uint32_t>(i);
            triangle.i1 = base + static_cast<std::uint32_t>(i + 1);
            triangle.i2 = base + static_cast<std::uint32_t>(i + 2);
            out_.triangles.push_back(triangle);
        }
    }

private:

    static const vsg::vec3Array* findPositions(
        const std::vector<vsg::ref_ptr<vsg::Data>>& arrays)
    {
        // vsgXchange кладёт POSITION первым массивом, но перестрахуемся
        for (const auto& data : arrays)
        {
            if (auto positions = data.cast<vsg::vec3Array>())
                return positions.get();
        }
        return nullptr;
    }

    std::uint32_t appendVertices(const vsg::vec3Array* positions)
    {
        const std::uint32_t base = static_cast<std::uint32_t>(out_.vertices.size());

        for (std::size_t i = 0; i < positions->size(); ++i)
        {
            const vsg::vec3& v = (*positions)[i];
            const vsg::dvec3 world = matrix_ * vsg::dvec3(v.x, v.y, v.z);
            out_.vertices.emplace_back(static_cast<float>(world.x),
                                       static_cast<float>(world.y),
                                       static_cast<float>(world.z));
        }

        return base;
    }

    template <typename IndexArray>
    static bool fillIndices(const IndexArray& array,
                            std::size_t first,
                            std::size_t count,
                            std::vector<std::uint32_t>& out)
    {
        if (first >= array.size())
            return false;

        const std::size_t last = std::min(first + count, array.size());
        out.reserve(out.size() + (last - first));
        for (std::size_t i = first; i < last; ++i)
            out.push_back(static_cast<std::uint32_t>(array[i]));

        return true;
    }

    static bool readIndices(const vsg::Data& data,
                            std::uint32_t first_index,
                            std::uint32_t index_count,
                            std::vector<std::uint32_t>& out)
    {
        const std::size_t first = first_index;

        if (auto indices = data.cast<vsg::uintArray>())
        {
            const std::size_t count = (index_count != 0) ? index_count : indices->size();
            return fillIndices(*indices, first, count, out);
        }
        if (auto indices = data.cast<vsg::ushortArray>())
        {
            const std::size_t count = (index_count != 0) ? index_count : indices->size();
            return fillIndices(*indices, first, count, out);
        }
        if (auto indices = data.cast<vsg::ubyteArray>())
        {
            const std::size_t count = (index_count != 0) ? index_count : indices->size();
            return fillIndices(*indices, first, count, out);
        }

        return false;
    }

    ExtractedMesh& out_;
    vsg::dmat4 matrix_;     // текущая матрица (по умолчанию - единичная)
};

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExtractedMesh extractModelGeometry(const std::string& filename)
{
    ExtractedMesh result;

    auto options = create_default_vsg_options();
    auto node = vsg::read(vsg::Path(filename), options);
    if (!node)
        return result;

    GeometryExtractor extractor(result);
    node->accept(extractor);

    auto compute_bounds = vsg::visit<vsg::ComputeBounds>(node);
    if (compute_bounds.bounds.valid())
        result.bounds = compute_bounds.bounds;

    result.valid = !result.triangles.empty();
    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool writeColmeshFile(const std::string& path, const ExtractedMesh& mesh)
{
    namespace fs = std::filesystem;

    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);

    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    const std::uint32_t vertex_count = static_cast<std::uint32_t>(mesh.vertices.size());
    const std::uint32_t triangle_count = static_cast<std::uint32_t>(mesh.triangles.size());

    file.write("RCM1", 4);
    file.write(reinterpret_cast<const char*>(&vertex_count), sizeof(vertex_count));
    file.write(reinterpret_cast<const char*>(&triangle_count), sizeof(triangle_count));
    file.write(reinterpret_cast<const char*>(mesh.vertices.data()),
               vertex_count * sizeof(collision::Vec3f));
    file.write(reinterpret_cast<const char*>(mesh.triangles.data()),
               triangle_count * sizeof(collision::Triangle));

    return file.good();
}
