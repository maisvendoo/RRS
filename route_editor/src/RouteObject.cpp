#include "RouteObject.h"

#include "EditorContext.h"
#include "Gizmo.h"
#include "Mask.h"
#include "Outline.h"
#include "SingleSwitch.h"

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

EditorContext* RouteObject::s_context = nullptr;

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
    return vsg::rotate(vsg::radians(rotation_deg.z), AXIS_Z_POSITIVE) *
           vsg::rotate(vsg::radians(rotation_deg.y), AXIS_Y_POSITIVE) *
           vsg::rotate(vsg::radians(rotation_deg.x), AXIS_X_POSITIVE);
}

RouteObject::RouteObject(
    EditorContext& context,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod,
    const std::string& label,
    vsg::vec3 translation,
    vsg::vec3 rotation_deg,
    vsg::vec3 scale
)
    : label(label)
    , translation(static_cast<vsg::dvec3>(translation))
    , rotation_deg(static_cast<vsg::dvec3>(rotation_deg))
    , scale(static_cast<vsg::vec3>(scale))
    , paged_lod(paged_lod)
{
    s_context = &context;

    paged_lod_switch = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch = SingleSwitch::create(vsg::MASK_OFF,
        nullptr);

    this->addChild(paged_lod_switch);
    this->addChild(outline_switch);

    update_matrix();
}

vsg::vec3 RouteObject::get_translation() const
{
    return static_cast<vsg::vec3>(translation);
}

vsg::vec3 RouteObject::get_rotation_deg() const
{
    return static_cast<vsg::vec3>(rotation_deg);
}

vsg::vec3 RouteObject::get_scale() const
{
    return static_cast<vsg::vec3>(scale);
}

const vsg::dmat4& RouteObject::get_initial_matrix() const
{
    return initial_matrix;
}

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

    this->matrix[3][0] = translation.x;
    this->matrix[3][1] = translation.y;
    this->matrix[3][2] = translation.z;

    update_bounds();
}

void RouteObject::set_rotation_deg(vsg::vec3 rotation_deg)
{
    this->rotation_deg = rotation_deg;
    update_matrix();
}

void RouteObject::set_scale(vsg::vec3 scale)
{
    this->scale = scale;
    update_matrix();
}

void RouteObject::move(vsg::vec3 translation)
{
    this->translation += static_cast<vsg::dvec3>(translation);

    this->matrix[3][0] += translation.x;
    this->matrix[3][1] += translation.y;
    this->matrix[3][2] += translation.z;

    update_bounds();
}

void RouteObject::rotate_around_pivot(vsg::vec3 pivot, vsg::vec3 axis,
    float radians, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) *
        vsg::rotate(vsg::quat(radians, axis)) * vsg::translate(-pivot) *
        static_cast<vsg::mat4>(matrix);

    vsg::vec3 temp_trans;
    vsg::quat temp_quat;
    vsg::vec3 temp_scale;
    vsg::decompose(static_cast<vsg::mat4>(this->matrix),
        temp_trans, temp_quat, temp_scale);

    this->translation = temp_trans;
    this->rotation_deg = to_euler_deg(temp_quat);
    this->scale = temp_scale;

    update_bounds();
}

void RouteObject::scale_relative_to_pivot(vsg::vec3 pivot,
    vsg::vec3 scale, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * vsg::scale(scale) *
        vsg::translate(-pivot) * static_cast<vsg::mat4>(matrix);

    vsg::vec3 temp_trans;
    vsg::quat temp_quat;
    vsg::vec3 temp_scale;
    vsg::decompose(static_cast<vsg::mat4>(this->matrix),
        temp_trans, temp_quat, temp_scale);

    this->translation = temp_trans;
    this->scale = temp_scale;

    update_bounds();
}

void RouteObject::hide()
{
    paged_lod_switch->mask = vsg::MASK_OFF;
    outline_switch->mask = vsg::MASK_OFF;

    is_hidden = true;

    s_context->hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden = false;

    RouteObjects& hidden_objects = s_context->hidden_objects;

    return hidden_objects.erase(std::find(hidden_objects.begin(),
        hidden_objects.end(), vsg::ref_ptr(this)));
}

bool RouteObject::select()
{
    if (!outline_switch->node)
    {
        const auto outline = s_context->outline_builder->create_outline(paged_lod);
        if (!outline)
        {
            return false;
        }

        s_context->compile_infos.emplace_back(
            CompileInfo{outline_switch, outline});
    }

    outline_switch->mask = MASK_GUI2;

    is_selected = true;

    s_context->selected_objects.emplace_back(this);
    s_context->gizmo->update_position();

    return true;
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch->mask = vsg::MASK_OFF;

    is_selected = false;

    RouteObjects& selected_objects = s_context->selected_objects;

    const auto it = selected_objects.erase(std::find(selected_objects.begin(),
        selected_objects.end(), vsg::ref_ptr(this)));

    s_context->gizmo->update_position();

    return it;
}

vsg::ref_ptr<RouteObject> RouteObject::copy() const
{
    return RouteObject::create(*s_context, paged_lod, label,
        static_cast<vsg::vec3>(translation), static_cast<vsg::vec3>(rotation_deg),
        static_cast<vsg::vec3>(scale));
}

void RouteObject::save_matrix()
{
    initial_matrix = this->matrix;
}

void RouteObject::set_matrix(const vsg::dmat4& matrix)
{
    this->matrix = matrix;

    vsg::vec3 temp_trans;
    vsg::quat temp_quat;
    vsg::vec3 temp_scale;
    vsg::decompose(static_cast<vsg::mat4>(this->matrix),
        temp_trans, temp_quat, temp_scale);

    this->translation = temp_trans;
    this->rotation_deg = to_euler_deg(temp_quat);
    this->scale = temp_scale;

    update_bounds();
}

void RouteObject::update_matrix()
{
    this->matrix = vsg::translate(static_cast<vsg::vec3>(this->translation)) *
        to_rotate_matrix(static_cast<vsg::vec3>(this->rotation_deg)) *
        vsg::scale(static_cast<vsg::vec3>(this->scale));

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    this->bounds = static_cast<vsg::box>(compute_bounds.bounds);

    s_context->gizmo->update_position();
}
