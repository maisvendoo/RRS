#include "RouteObject.h"

#include "Gizmo.h"
#include "Mask.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include "SingleSwitch.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>
#include <vsg/core/observer_ptr.h>
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
#include <cassert>
#include <cmath>
#include <string>

static constexpr vsg::vec3 AXIS_X_POSITIVE = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Y_POSITIVE = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Z_POSITIVE = {0.0f, 0.0f, 1.0f};

static vsg::vec3 to_euler_deg(const vsg::quat& quat)
{
    return vsg::vec3{
        vsg::degrees(std::atan2(2.0f * (quat.w * quat.x + quat.y * quat.z),
            1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y))),
        vsg::degrees(std::asin(2.0f * (quat.w * quat.y - quat.z * quat.x))),
        vsg::degrees(std::atan2(2.0f * (quat.w * quat.z + quat.x * quat.y),
            1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z)))
    };
}

static vsg::mat4 to_rotate_matrix(vsg::vec3 rotation_deg)
{
    const vsg::vec3 rotation_rad = {
        vsg::radians(rotation_deg.x),
        vsg::radians(rotation_deg.y),
        vsg::radians(rotation_deg.z)
    };

    const vsg::mat4 rotate_x = vsg::rotate(rotation_rad.x, AXIS_X_POSITIVE);
    const vsg::mat4 rotate_y = vsg::rotate(rotation_rad.y, AXIS_Y_POSITIVE);
    const vsg::mat4 rotate_z = vsg::rotate(rotation_rad.z, AXIS_Z_POSITIVE);

    return rotate_z * rotate_y * rotate_x;
}

RouteObject::RouteObject(EditorContext& context, vsg::ref_ptr<vsg::PagedLOD> paged_lod,
    const std::string& label, vsg::vec3 translation, vsg::vec3 rotation_deg)
    : label(label)
    , context(context)
    , translation(translation)
    , rotation_deg(rotation_deg)
{
    assert(paged_lod);

    update_matrix();

    paged_lod_switch = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch = SingleSwitch::create(vsg::MASK_OFF,
        nullptr);

    this->addChild(paged_lod_switch);
    this->addChild(outline_switch);
}

vsg::vec3 RouteObject::get_translation() const
{
    return translation;
}

vsg::vec3 RouteObject::get_rotation_deg() const
{
    return rotation_deg;
}

vsg::vec3 RouteObject::get_scale() const
{
    return scale_value;
}

// vsg::vec3 RouteObject::get_initial_translation() const
// {
//     return initial_translation;
// }

// vsg::vec3 RouteObject::get_initial_rotation_deg() const
// {
//     return initial_rotation_deg;
// }

// vsg::vec3 RouteObject::get_initial_scale() const
// {
//     return initial_scale;
// }

// const vsg::dmat4& RouteObject::get_initial_matrix() const
// {
//     return initial_matrix;
// }

const vsg::box& RouteObject::get_bounds() const
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

void RouteObject::set_translation(vsg::vec3 translation)
{
    this->translation = translation;

    this->matrix[3][0] = this->translation.x;
    this->matrix[3][1] = this->translation.y;
    this->matrix[3][2] = this->translation.z;

    update_bounds();
}

void RouteObject::set_rotation_deg(vsg::vec3 rotation_deg, bool update_matrix)
{
    this->rotation_deg = rotation_deg;

    if (update_matrix)
    {
        this->update_matrix();
    }
}

void RouteObject::set_scale(vsg::vec3 scale, bool update_matrix)
{
    this->scale_value = scale;

    if (update_matrix)
    {
        this->update_matrix();
    }
}

void RouteObject::move(vsg::vec3 translation)
{
    this->translation += translation;

    this->matrix[3][0] = this->translation.x;
    this->matrix[3][1] = this->translation.y;
    this->matrix[3][2] = this->translation.z;

    update_bounds();
}

void RouteObject::rotate(vsg::vec3 rotation_deg, bool update_matrix)
{
    this->rotation_deg += rotation_deg;

    if (update_matrix)
    {
        this->update_matrix();
    }
}

void RouteObject::scale(vsg::vec3 scale, bool update_matrix)
{
    this->scale_value *= scale;

    if (update_matrix)
    {
        this->update_matrix();
    }
}

void RouteObject::rotate_around_pivot(vsg::vec3 pivot,
    vsg::vec3 rotation_deg, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * to_rotate_matrix(rotation_deg) *
        vsg::translate(-pivot) * static_cast<vsg::mat4>(matrix);

    vsg::quat quat;

    vsg::decompose(static_cast<vsg::mat4>(this->matrix),
        translation, quat, scale_value);

    this->rotation_deg = to_euler_deg(quat);

    update_bounds();
}

void RouteObject::scale_relative_to_pivot(vsg::vec3 pivot,
    vsg::vec3 scale, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * vsg::scale(scale) *
        vsg::translate(-pivot) * static_cast<vsg::mat4>(matrix);

    vsg::quat quat;

    vsg::decompose(static_cast<vsg::mat4>(this->matrix),
        translation, quat, scale_value);

    update_bounds();
}

void RouteObject::hide()
{
    paged_lod_switch->mask = vsg::MASK_OFF;
    outline_switch->mask = vsg::MASK_OFF;

    is_hidden = true;

    context.hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden = false;

    return context.hidden_objects.erase(std::find(context.hidden_objects.cbegin(),
        context.hidden_objects.cend(), this));
}

void RouteObject::select()
{
    if (!outline_switch->node)
    {
        const auto compile_manager = context.viewer->compileManager;
        const auto outline = context.outline_builder->create_outline(paged_lod_switch->node.cast<vsg::PagedLOD>());
        const auto compile_result = compile_manager->compile(outline);
        outline_switch->node = outline;
        vsg::updateViewer(*context.viewer, compile_result);
    }

    outline_switch->mask = MASK_GUI2;

    is_selected = true;

    context.selected_objects.emplace_back(this);

    context.gizmo->update_position();
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch->mask = vsg::MASK_OFF;

    is_selected = false;

    const auto it = context.selected_objects.erase(std::find(
        context.selected_objects.cbegin(), context.selected_objects.cend(), this));

    context.gizmo->update_position();

    return it;
}

// void RouteObject::save_translation()
// {
//     initial_translation = translation;
// }

// void RouteObject::save_rotation()
// {
//     initial_rotation_deg = rotation_deg;
// }

// void RouteObject::save_scale()
// {
//     initial_scale = scale_value;
// }

// void RouteObject::save_matrix()
// {
//     initial_matrix = this->matrix;
// }

void RouteObject::update_matrix()
{
    this->matrix = vsg::translate(translation) *
        to_rotate_matrix(rotation_deg) * vsg::scale(scale_value);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds = static_cast<vsg::box>(compute_bounds.bounds);

    context.gizmo->update_position();
}
