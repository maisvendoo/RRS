#include "editor/ObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <cstddef>

ObjectManager::ObjectManager(std::size_t max_object_count)
    : max_object_count(max_object_count)
{
    paged_lods.reserve(max_object_count);
    transforms.reserve(max_object_count);
}

ObjectManager::~ObjectManager() = default;

void ObjectManager::add_paged_lod(const vsg::ref_ptr<vsg::PagedLOD>& paged_lod)
{
    paged_lods.push_back(paged_lod);
}

void ObjectManager::add_matrix_transform(const vsg::ref_ptr<vsg::MatrixTransform>& matrix_transform)
{
    transforms.push_back(matrix_transform);
}

const std::vector<vsg::ref_ptr<vsg::MatrixTransform>>& ObjectManager::get_transforms() const
{
    return transforms;
}
