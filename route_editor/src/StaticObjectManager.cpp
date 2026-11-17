#include "editor/StaticObjectManager.h"

#include "editor/EditorContext.h"
#include "editor/Mask.h"
#include "editor/MergeSwitch.h"
#include "editor/Outline.h"
#include "editor/Route.h"

#include <Journal.h>

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Switch.h>

#include <QString>

#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

// static constexpr vsg::vec3 X_AXISf = {1.0f, 0.0f, 0.0f};
// static constexpr vsg::vec3 Y_AXISf = {0.0f, 1.0f, 0.0f};
// static constexpr vsg::vec3 Z_AXISf = {0.0f, 0.0f, 1.0f};

static constexpr vsg::dvec3 X_AXIS = {1.0, 0.0, 0.0};
static constexpr vsg::dvec3 Y_AXIS = {0.0, 1.0, 0.0};
static constexpr vsg::dvec3 Z_AXIS = {0.0, 0.0, 1.0};

StaticObjectManager::StaticObjectManager(EditorContext& editor_ctx,
                                         std::size_t max_object_count)
    : editor_ctx(editor_ctx)
    , labels(max_object_count)
    , rel_paths(max_object_count)
    , mat_transforms(max_object_count)
    , vsg_switches(max_object_count)
    , paged_lod_switches(max_object_count)
    , outline_switches(max_object_count, nullptr)
    , paged_lods(max_object_count)
    , init_mats(max_object_count)
    , selected_objects(max_object_count)
    , hidden_objects(max_object_count)
    , object_count(0)
    , max_object_count(max_object_count)
{
}

// TODO: Add multithreading
bool StaticObjectManager::add_object(
    const std::string& label, const std::string& rel_path,
    const vsg::ref_ptr<vsg::PagedLOD>& paged_lod, const vsg::dvec3& pos,
    const vsg::dvec3& rot_deg, const vsg::dvec3& scale)
{
    if (object_count >= max_object_count)
    {
        Journal::instance()->error(
            QString("Could not add more static objects.\n"
                    "Max static object count: %1").arg(max_object_count));

        return false;
    }

    labels[object_count] = label;
    rel_paths[object_count] = rel_path;

    const auto vsg_switch = vsg::Switch::create();
    vsg_switch->addChild(vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    vsg_switches[object_count] = vsg_switch;
    paged_lod_switches[object_count] = &vsg_switch->children[0];

    const auto mat_transform = vsg::MatrixTransform::create();
    mat_transform->addChild(vsg_switch);
    mat_transform->matrix = vsg::translate(pos) *
                            vsg::rotate(vsg::radians(rot_deg.z), Z_AXIS) *
                            vsg::rotate(vsg::radians(rot_deg.y), Y_AXIS) *
                            vsg::rotate(vsg::radians(rot_deg.x), X_AXIS) *
                            vsg::scale(scale);

    mat_transforms[object_count] = mat_transform;
    paged_lods[object_count] = paged_lod;

    const auto& viewer = editor_ctx.viewer;
    const vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
    const auto compile_result = viewer->compileManager->compile(mat_transform);

    viewer->addUpdateOperation(MergeSwitch::create(observer_viewer,
                                                   editor_ctx.route,
                                                   vsg::MASK_ALL,
                                                   mat_transform,
                                                   compile_result));

    mat_transform_to_id[mat_transform.get()] = object_count;

    ++object_count;

    return true;
}

void StaticObjectManager::remove_object(std::size_t id)
{
    assert((object_count > 0) && "Could not call remove_object() when "
                                 "object count is 0");
    assert((id < object_count) && "id must be lower than object count");

    const std::size_t last_object_id = object_count - 1;
    if (id == last_object_id)
    {
        mat_transform_to_id.erase(
            mat_transform_to_id.find(mat_transforms[id].get()));
        --object_count;
        return;
    }

    labels[id] = std::move(labels[last_object_id]);
    rel_paths[id] = std::move(rel_paths[last_object_id]);

    mat_transforms[id] = mat_transforms[last_object_id];
    vsg_switches[id] = vsg_switches[last_object_id];
    paged_lod_switches[id] = paged_lod_switches[last_object_id];
    outline_switches[id] = outline_switches[last_object_id];
    paged_lods[id] = paged_lods[last_object_id];

    mat_transform_to_id.erase(
        mat_transform_to_id.find(mat_transforms[id].get()));
    mat_transform_to_id[mat_transforms[last_object_id].get()] = id;

    selected_objects.remove_if_contains(id);
    hidden_objects.remove_if_contains(id);

    --object_count;
}

bool StaticObjectManager::select_object(std::size_t id)
{
    assert((object_count > 0) && "Could not call select_object() when "
                                 "object count is 0");
    assert((id < object_count) && "id must be lower than object count");

    if (selected_objects.contains(id))
    {
        return true;
    }

    // May not work properly if called again while adding update operation to
    // viewer
    if (!outline_switches[id])
    {
        const auto& outline_builder = editor_ctx.outline_builder;

        const auto outline = outline_builder->create_outline(paged_lods[id]);
        if (!outline)
        {
            return false;
        }

        const auto& viewer = editor_ctx.viewer;
        vsg::observer_ptr<vsg::Viewer> observer_viewer(viewer);
        const auto compile_result = viewer->compileManager->compile(outline);

        viewer->addUpdateOperation(MergeSwitch::create(
            observer_viewer, vsg_switches[id], vsg::Mask{MASK_GUI2}, outline,
            compile_result, [&, id]() -> void {
                outline_switches[id] = &vsg_switches[id]->children[1];
                selected_objects.add(id);
            }
        ));

        return true;
    }

    outline_switches[id]->mask = MASK_GUI2;
    selected_objects.add(id);

    return true;
}

bool StaticObjectManager::select_object(
    const vsg::MatrixTransform* mat_transform)
{
    const auto found_it = mat_transform_to_id.find(mat_transform);
    if (found_it == mat_transform_to_id.cend())
    {
        return false;
    }
    else
    {
        return select_object(found_it->second);
    }
}

void StaticObjectManager::deselect_object(std::size_t id)
{
    assert((object_count > 0) && "Could not call deselect_object() when "
                                 "object count is 0");
    assert((id < object_count) && "id must be lower than object count");

    if (selected_objects.contains(id))
    {
        outline_switches[id]->mask = vsg::MASK_OFF;
        selected_objects.remove(id);
    }
}

void StaticObjectManager::set_position(std::size_t id, const vsg::dvec3& pos)
{
    assert((object_count > 0) && "Could not call set_position() when "
                                 "object count is 0");
    assert((id < object_count) && "id must be lower than object count");

    auto& matrix = mat_transforms[id]->matrix;
    matrix[3][0] = pos.x;
    matrix[3][1] = pos.y;
    matrix[3][2] = pos.z;
}

void StaticObjectManager::move_object(std::size_t id,
                                      const vsg::dvec3& delta_pos)
{
    assert((object_count > 0) && "Could not call move() when "
                                 "object count is 0");
    assert((id < object_count) && "id must be lower than object count");

    auto& matrix = mat_transforms[id]->matrix;
    matrix[3][0] += delta_pos.x;
    matrix[3][1] += delta_pos.y;
    matrix[3][2] += delta_pos.z;
}

const SparseSet& StaticObjectManager::get_selected_objects() const
{
    return selected_objects;
}

std::size_t StaticObjectManager::get_object_count() const
{
    return object_count;
}
