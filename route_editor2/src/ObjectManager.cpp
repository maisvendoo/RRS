#include "editor/ObjectManager.h"

#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Object.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec4.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/VertexIndexDraw.h>

#include <cstddef>
#include <string>

class FindArraysVisitor : public vsg::Inherit<vsg::Visitor, FindArraysVisitor>
{
public:
    vsg::ref_ptr<vsg::vec3Array> positions;
    vsg::ref_ptr<vsg::ushortArray> ushort_indices;
    vsg::ref_ptr<vsg::uintArray> uint_indices;

public:
    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::VertexIndexDraw& vid) override
    {
        for (const auto& buffer_info : vid.arrays)
        {
            const auto data = buffer_info->data;
            if (const auto array = data->cast<vsg::vec3Array>())
            {
                positions = array;
            }
        }

        ushort_indices = vid.indices->data->cast<vsg::ushortArray>();
        uint_indices = vid.indices->data->cast<vsg::uintArray>();
    }

    vsg::ref_ptr<vsg::Data> get_indices() const
    {
        if (ushort_indices)
        {
            return ushort_indices;
        }
        else
        {
            return uint_indices;
        }
    }

    std::size_t get_indices_size() const
    {
        if (ushort_indices)
        {
            return ushort_indices->size();
        }
        else
        {
            return uint_indices->size();
        }
    }
};

ObjectManager::ObjectManager(std::size_t max_object_count)
    : max_object_count(max_object_count)
{
    labels.reserve(max_object_count);
    paths.reserve(max_object_count);
    paged_lods.reserve(max_object_count);
    transforms.reserve(max_object_count);
}

ObjectManager::~ObjectManager() = default;

void ObjectManager::push_label(const std::string& label)
{
    labels.push_back(label);
}

void ObjectManager::push_path(const std::string& path)
{
    paths.push_back(path);
}

void ObjectManager::push_paged_lod(const vsg::ref_ptr<vsg::PagedLOD>& paged_lod)
{
    paged_lods.push_back(paged_lod);
}

void ObjectManager::push_matrix_transform(const vsg::ref_ptr<vsg::MatrixTransform>& matrix_transform)
{
    transforms.push_back(matrix_transform);
}

const std::vector<vsg::ref_ptr<vsg::MatrixTransform>>& ObjectManager::get_transforms() const
{
    return transforms;
}

vsg::ref_ptr<vsg::Geometry> ObjectManager::create_geometry_for_selection_buffer() const
{
    std::size_t count = 0;

    const std::size_t paged_lods_size = paged_lods.size();
    for (std::size_t i = 0; i < paged_lods_size; ++i)
    {
        const auto& node = paged_lods[i]->pending;
        if (!node)
        {
            continue;
        }

        FindArraysVisitor fav;
        node->accept(fav);

        const float r = (i << 0) & 0xFF;
        const float g = (i << 8) & 0xFF;
        const float b = (i << 16) & 0xFF;
        const float a = (i << 24) & 0xFF;

        const auto colors = vsg::vec4Array::create(fav.positions->size(),
            vsg::vec4(r, g, b, a));

        ++count;
    }

    printf("count: %zu\n", count);

    return {};
}
