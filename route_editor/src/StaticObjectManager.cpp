#include "editor/StaticObjectManager.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Switch.h>

#include <cstddef>
#include <string>
#include <utility>

StaticObjectManager::StaticObjectManager(EditorContext& editor_ctx,
                                         std::size_t max_object_count)
    : editor_ctx(editor_ctx)
    , labels(max_object_count)
    , rel_paths(max_object_count)
    , paged_lods(max_object_count)
    , mat_transforms(max_object_count)
    , object_switches(max_object_count)
    , init_mats(max_object_count)
    , is_selected(max_object_count)
    , is_hidden(max_object_count)
    , object_count(0)
    , max_object_count(max_object_count)
{
}

bool StaticObjectManager::add_object(
    const std::string& label, const std::string& rel_path,
    const vsg::ref_ptr<vsg::PagedLOD>& paged_lod, const vsg::dvec3& pos,
    const vsg::dvec3& rot_deg, const vsg::dvec3& scale, bool is_selected,
    bool is_hidden)
{
    if (object_count >= max_object_count)
    {
        return false;
    }

    labels.at(object_count) = label;
    rel_paths.at(object_count) = rel_path;
    paged_lods.at(object_count) = paged_lod;

    // const auto outline =

    // mat_transforms.at(object_count) = mat_transform;
    this->is_selected.at(object_count) = is_selected;
    this->is_hidden.at(object_count) = is_hidden;
    ++object_count;

    return true;
}

bool StaticObjectManager::remove_object(std::size_t id)
{
    if (id >= object_count)
    {
        return false;
    }

    const std::size_t last_object_id = object_count - 1;
    if (id == last_object_id)
    {
        --object_count;
        return true;
    }

    labels.at(id) = std::move(labels.at(last_object_id));
    rel_paths.at(id) = std::move(rel_paths.at(last_object_id));
    mat_transforms.at(id) = std::move(mat_transforms.at(last_object_id));
    is_selected.at(id) = is_selected.at(last_object_id);
    is_hidden.at(id) = is_hidden.at(last_object_id);
    --object_count;

    return true;
}
