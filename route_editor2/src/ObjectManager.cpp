#include "editor/ObjectManager.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>

#include <cstddef>
#include <string>

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

void ObjectManager::check_intersections_and_select_objects(const vsg::ref_ptr<Camera>& camera,
    int x1, int y1, int x2, int y2)
{
}
