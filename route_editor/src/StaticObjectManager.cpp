#include "editor/StaticObjectManager.h"

#include <vsg/nodes/MatrixTransform.h>

#include <cstddef>

StaticObjectManager::StaticObjectManager(std::size_t max_object_count)
    : object_count(0)
    , max_object_count(max_object_count)
{
    labels.reserve(max_object_count);
    rel_paths.reserve(max_object_count);
    mat_transforms.reserve(max_object_count);
    init_mats.reserve(max_object_count);
    is_selected.reserve(max_object_count);
    is_hidden.reserve(max_object_count);
}

bool StaticObjectManager::add_object(
    const std::string& label, const std::string& rel_path,
    const vsg::ref_ptr<vsg::MatrixTransform>& mat_transform, bool is_selected,
    bool is_hidden)
{
    if (object_count >= max_object_count)
    {
        return false;
    }

    labels.emplace_back(label);
    rel_paths.emplace_back(rel_path);
    mat_transforms.emplace_back(mat_transform);
    this->is_selected.emplace_back(is_selected);
    this->is_hidden.emplace_back(is_hidden);
    ++object_count;

    return true;
}

bool StaticObjectManager::remove_object(std::size_t idx)
{
    return true;
}
