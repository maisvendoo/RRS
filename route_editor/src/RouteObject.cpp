#include "editor/RouteObject.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Mask.h"
#include "editor/Outline.h"
#include "editor/SingleSwitch.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/quat.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/utils/ComputeBounds.h>

#include <algorithm>
#include <cmath>
#include <string>

static vsg::dvec3 to_euler_deg(const vsg::dquat& q)
{
    return vsg::dvec3{
        vsg::degrees(std::atan2(2.0 * (q.w * q.x + q.y * q.z),
                                1.0 - 2.0 * (q.x * q.x + q.y * q.y))),
        vsg::degrees(std::asin(2.0 * (q.w * q.y - q.z * q.x))),
        vsg::degrees(std::atan2(2.0 * (q.w * q.z + q.x * q.y),
                                1.0 - 2.0 * (q.y * q.y + q.z * q.z)))};
}

RouteObject::RouteObject(EditorContext& editor_ctx,
                         const vsg::ref_ptr<vsg::PagedLOD>& paged_lod,
                         const std::string& label, const vsg::dvec3& position,
                         const vsg::dvec3& rot_deg, const vsg::dvec3& scale)
    : label(label)
    , editor_ctx(editor_ctx)
    , position(position)
    , rot_deg(rot_deg)
    , scale(scale)
    , paged_lod(paged_lod)
{
    paged_lod_switch = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch = SingleSwitch::create(vsg::MASK_OFF, nullptr);

    this->addChild(paged_lod_switch);
    this->addChild(outline_switch);

    update_matrix();
}

const vsg::dvec3& RouteObject::get_position() const
{
    return position;
}

const vsg::dvec3& RouteObject::get_rot_deg() const
{
    return rot_deg;
}

const vsg::dvec3& RouteObject::get_scale() const
{
    return scale;
}

const vsg::dmat4& RouteObject::get_initial_matrix() const
{
    return initial_matrix;
}

const vsg::dbox& RouteObject::get_bounds() const
{
    return bounds;
}

bool RouteObject::get_is_selected() const
{
    return is_selected;
}

bool RouteObject::get_is_hidden() const
{
    return is_hidden;
}

void RouteObject::set_position(const vsg::dvec3& position)
{
    this->position = position;

    matrix[3][0] = position.x;
    matrix[3][1] = position.y;
    matrix[3][2] = position.z;

    update_bounds();
}

void RouteObject::set_rot_deg(const vsg::dvec3& rot_deg)
{
    this->rot_deg = rot_deg;
    update_matrix();
}

void RouteObject::set_scale(const vsg::dvec3& scale)
{
    this->scale = scale;
    update_matrix();
}

void RouteObject::move(const vsg::dvec3& delta_position)
{
    set_position(position + delta_position);
}

void RouteObject::rotate_around_pivot(const vsg::dvec3& pivot,
                                      const vsg::dvec3& axis, double radians,
                                      const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) *
                   vsg::rotate(vsg::dquat(radians, axis)) *
                   vsg::translate(-pivot) * matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::scale_relative_to_pivot(const vsg::dvec3& pivot,
                                          const vsg::dvec3& scale,
                                          const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * vsg::scale(scale)
                   * vsg::translate(-pivot) * matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::hide()
{
    paged_lod_switch->mask = vsg::MASK_OFF;
    outline_switch->mask = vsg::MASK_OFF;

    is_hidden = true;

    editor_ctx.hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden = false;

    RouteObjects& hidden_objects = editor_ctx.hidden_objects;

    return hidden_objects.erase(std::find(
        hidden_objects.begin(), hidden_objects.end(), vsg::ref_ptr(this)));
}

bool RouteObject::select()
{
    if (!outline_switch->node)
    {
        auto outline
            = editor_ctx.outline_builder->create_outline(paged_lod);
        if (!outline)
        {
            return false;
        }

        editor_ctx.compile_infos.lock()->emplace_back(
            CompileInfo{outline_switch, outline});
    }

    outline_switch->mask = MASK_GUI2;

    is_selected = true;

    editor_ctx.selected_objects.emplace_back(this);
    editor_ctx.gizmo->update_position();

    return true;
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch->mask = vsg::MASK_OFF;

    is_selected = false;

    auto& selected_objects = editor_ctx.selected_objects;

    const auto it = selected_objects.erase(std::find(
        selected_objects.begin(), selected_objects.end(), vsg::ref_ptr(this)));

    editor_ctx.gizmo->update_position();

    return it;
}

vsg::ref_ptr<RouteObject> RouteObject::copy() const
{
    return RouteObject::create(editor_ctx, paged_lod, label, position, rot_deg,
                               scale);
}

void RouteObject::save_matrix()
{
    initial_matrix = matrix;
}

void RouteObject::set_matrix(const vsg::dmat4& matrix)
{
    this->matrix = matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::update_matrix()
{
    constexpr vsg::dvec3 X_AXIS = {1.0, 0.0, 0.0};
    constexpr vsg::dvec3 Y_AXIS = {0.0, 1.0, 0.0};
    constexpr vsg::dvec3 Z_AXIS = {0.0, 0.0, 1.0};

    matrix = vsg::translate(position) *
             vsg::rotate(vsg::radians(rot_deg.z), Z_AXIS) *
             vsg::rotate(vsg::radians(rot_deg.y), Y_AXIS) *
             vsg::rotate(vsg::radians(rot_deg.x), X_AXIS) *
             vsg::scale(scale);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds = compute_bounds.bounds;

    editor_ctx.gizmo->update_position();
}

void RouteObject::decompose_matrix()
{
    vsg::dquat temp_quat;
    vsg::decompose(matrix, position, temp_quat, scale);
    rot_deg = to_euler_deg(temp_quat);
}
