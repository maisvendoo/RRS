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

EditorContext* RouteObject::s_context_ = nullptr;

static constexpr vsg::dvec3 AXIS_X_POSITIVE = {1.0, 0.0, 0.0};
static constexpr vsg::dvec3 AXIS_Y_POSITIVE = {0.0, 1.0, 0.0};
static constexpr vsg::dvec3 AXIS_Z_POSITIVE = {0.0, 0.0, 1.0};

static vsg::dvec3 to_euler_deg(const vsg::dquat& quat)
{
    return vsg::dvec3{
        vsg::degrees(std::atan2(2.0 * (quat.w * quat.x + quat.y * quat.z),
            1.0 - 2.0 * (quat.x * quat.x + quat.y * quat.y))),
        vsg::degrees(std::asin(2.0 * (quat.w * quat.y - quat.z * quat.x))),
        vsg::degrees(std::atan2(2.0 * (quat.w * quat.z + quat.x * quat.y),
            1.0 - 2.0 * (quat.y * quat.y + quat.z * quat.z)))
    };
}

static vsg::dmat4 to_rotate_matrix(const vsg::dvec3& rotation_deg)
{
    return vsg::rotate(vsg::radians(rotation_deg.z), AXIS_Z_POSITIVE) *
           vsg::rotate(vsg::radians(rotation_deg.y), AXIS_Y_POSITIVE) *
           vsg::rotate(vsg::radians(rotation_deg.x), AXIS_X_POSITIVE);
}

RouteObject::RouteObject(
    EditorContext& context,
    vsg::ref_ptr<vsg::PagedLOD> paged_lod,
    const std::string& label,
    const vsg::dvec3& translation,
    const vsg::dvec3& rotation_deg,
    const vsg::dvec3& scale
)
    : label(label)
    , translation_(translation)
    , rotation_deg_(rotation_deg)
    , scale_(scale)
    , paged_lod_(paged_lod)
{
    s_context_ = &context;

    paged_lod_switch_ = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch_ = SingleSwitch::create(vsg::MASK_OFF,
        nullptr);

    this->addChild(paged_lod_switch_);
    this->addChild(outline_switch_);

    update_matrix();
}

const vsg::dvec3& RouteObject::get_translation() const
{
    return translation_;
}

const vsg::dvec3& RouteObject::get_rotation_deg() const
{
    return rotation_deg_;
}

const vsg::dvec3& RouteObject::get_scale() const
{
    return scale_;
}

const vsg::dmat4& RouteObject::get_initial_matrix() const
{
    return initial_matrix_;
}

const vsg::dbox& RouteObject::get_bounds() const
{
    return bounds_;
}

bool RouteObject::get_is_selected() const
{
    return is_selected_;
}

bool RouteObject::get_is_hidden() const
{
    return is_hidden_;
}

void RouteObject::set_translation(const vsg::dvec3& translation)
{
    translation_ = translation;

    matrix[3][0] = translation.x;
    matrix[3][1] = translation.y;
    matrix[3][2] = translation.z;

    update_bounds();
}

void RouteObject::set_rotation_deg(const vsg::dvec3& rotation_deg)
{
    rotation_deg_ = rotation_deg;
    update_matrix();
}

void RouteObject::set_scale(const vsg::dvec3& scale)
{
    scale_ = scale;
    update_matrix();
}

void RouteObject::move(const vsg::dvec3& translation)
{
    set_translation(translation_ + translation);
}

void RouteObject::rotate_around_pivot(const vsg::dvec3& pivot, const vsg::dvec3& axis,
    double radians, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * vsg::rotate(vsg::dquat(radians, axis)) *
        vsg::translate(-pivot) * matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::scale_relative_to_pivot(const vsg::dvec3& pivot,
    const vsg::dvec3& scale, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) * vsg::scale(scale) *
        vsg::translate(-pivot) * matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::hide()
{
    paged_lod_switch_->mask = vsg::MASK_OFF;
    outline_switch_->mask = vsg::MASK_OFF;

    is_hidden_ = true;

    s_context_->hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch_->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden_ = false;

    RouteObjects& hidden_objects = s_context_->hidden_objects;

    return hidden_objects.erase(std::find(hidden_objects.begin(),
        hidden_objects.end(), vsg::ref_ptr(this)));
}

bool RouteObject::select()
{
    if (!outline_switch_->node)
    {
        const auto outline = s_context_->outline_builder->create_outline(paged_lod_);
        if (!outline)
        {
            return false;
        }

        s_context_->compile_infos.emplace_back(
            CompileInfo{outline_switch_, outline});
    }

    outline_switch_->mask = MASK_GUI2;

    is_selected_ = true;

    s_context_->selected_objects.emplace_back(this);
    s_context_->gizmo->update_position();

    return true;
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch_->mask = vsg::MASK_OFF;

    is_selected_ = false;

    RouteObjects& selected_objects = s_context_->selected_objects;

    const auto it = selected_objects.erase(std::find(selected_objects.begin(),
        selected_objects.end(), vsg::ref_ptr(this)));

    s_context_->gizmo->update_position();

    return it;
}

vsg::ref_ptr<RouteObject> RouteObject::copy() const
{
    return RouteObject::create(*s_context_, paged_lod_, label,
        translation_, rotation_deg_, scale_);
}

void RouteObject::save_matrix()
{
    initial_matrix_ = matrix;
}

void RouteObject::set_matrix(const vsg::dmat4& matrix)
{
    this->matrix = matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::update_matrix()
{
    matrix = vsg::translate(translation_) * to_rotate_matrix(rotation_deg_) *
        vsg::scale(scale_);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds_ = compute_bounds.bounds;

    s_context_->gizmo->update_position();
}

void RouteObject::decompose_matrix()
{
    vsg::dquat temp_quat;
    vsg::decompose(matrix, translation_, temp_quat, scale_);
    rotation_deg_ = to_euler_deg(temp_quat);
}
