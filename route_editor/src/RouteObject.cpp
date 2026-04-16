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

static constexpr vsg::dvec3 AXIS_X_POSITIVE = {1.0, 0.0, 0.0};
static constexpr vsg::dvec3 AXIS_Y_POSITIVE = {0.0, 1.0, 0.0};
static constexpr vsg::dvec3 AXIS_Z_POSITIVE = {0.0, 0.0, 1.0};

static vsg::dvec3 to_euler_deg(const vsg::dquat& q)
{
    return vsg::dvec3{
        vsg::degrees(std::atan2(2.0 * (q.w * q.x + q.y * q.z),
            1.0 - 2.0 * (q.x * q.x + q.y * q.y))),
        vsg::degrees(std::asin(2.0 * (q.w * q.y - q.z * q.x))),
        vsg::degrees(std::atan2(2.0 * (q.w * q.z + q.x * q.y),
            1.0 - 2.0 * (q.y * q.y + q.z * q.z)))
    };
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
    , context_(context)
    , translation_(translation)
    , rotation_deg_(rotation_deg)
    , scale_(scale)
    , paged_lod_(paged_lod)
{
    paged_lod_switch_ = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch_ = SingleSwitch::create(vsg::MASK_OFF, nullptr);

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

void RouteObject::rotate_around_pivot(const vsg::dvec3& pivot,
    const vsg::dvec3& axis, double radians, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) *
                   vsg::rotate(vsg::dquat(radians, axis)) *
                   vsg::translate(-pivot) * matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::scale_relative_to_pivot(const vsg::dvec3& pivot,
    const vsg::dvec3& scale, const vsg::dmat4& matrix)
{
    this->matrix = vsg::translate(pivot) *
                   vsg::scale(scale) *
                   vsg::translate(-pivot) *
                   matrix;

    decompose_matrix();
    update_bounds();
}

void RouteObject::hide()
{
    paged_lod_switch_->mask = vsg::MASK_OFF;
    outline_switch_->mask = vsg::MASK_OFF;

    is_hidden_ = true;

    context_.hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch_->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden_ = false;

    RouteObjects& hidden_objects = context_.hidden_objects;

    return hidden_objects.erase(std::find(hidden_objects.begin(),
        hidden_objects.end(), vsg::ref_ptr(this)));
}

bool RouteObject::select()
{
    if (!outline_switch_->node)
    {
        const auto outline = context_.outline_builder->create_outline(
            paged_lod_);

        if (!outline)
        {
            return false;
        }

        std::lock_guard<std::mutex> lock_guard(context_.compile_infos_mutex);
        context_.compile_infos.emplace_back(
            CompileInfo{outline_switch_, outline});
    }

    outline_switch_->mask = MASK_GUI2;

    is_selected_ = true;

    context_.selected_objects.emplace_back(this);
    context_.gizmo->update_position();

    return true;
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch_->mask = vsg::MASK_OFF;

    is_selected_ = false;

    RouteObjects& selected_objects = context_.selected_objects;

    const auto it = selected_objects.erase(std::find(selected_objects.begin(),
        selected_objects.end(), vsg::ref_ptr(this)));

    context_.gizmo->update_position();

    return it;
}

vsg::ref_ptr<RouteObject> RouteObject::copy() const
{
    return RouteObject::create(context_, paged_lod_, label,
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
    matrix = vsg::translate(translation_) *
             vsg::rotate(vsg::radians(rotation_deg_.z), AXIS_Z_POSITIVE) *
             vsg::rotate(vsg::radians(rotation_deg_.y), AXIS_Y_POSITIVE) *
             vsg::rotate(vsg::radians(rotation_deg_.x), AXIS_X_POSITIVE) *
             vsg::scale(scale_);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds_ = compute_bounds.bounds;

    context_.gizmo->update_position();
}

void RouteObject::decompose_matrix()
{
    vsg::dquat temp_quat;
    vsg::decompose(matrix, translation_, temp_quat, scale_);
    rotation_deg_ = to_euler_deg(temp_quat);
}
