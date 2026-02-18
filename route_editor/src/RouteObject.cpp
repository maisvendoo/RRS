#include "RouteObject.h"

#include "Mask.h"
#include "Outline.h"
#include "SingleSwitch.h"

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

vsg::observer_ptr<vsg::Viewer> RouteObject::s_observer_viewer;
RouteObjects RouteObject::s_selected_objects;
RouteObjects RouteObject::s_hidden_objects;

static constexpr vsg::vec3 AXIS_X_POSITIVE = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Y_POSITIVE = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Z_POSITIVE = {0.0f, 0.0f, 1.0f};

static vsg::vec3 quat_to_euler(const vsg::quat& quat)
{
    return {
        vsg::degrees(std::atan2(2.0f * (quat.w * quat.x + quat.y * quat.z),
            1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y))),
        vsg::degrees(std::asin(2.0f * (quat.w * quat.y - quat.z * quat.x))),
        vsg::degrees(std::atan2(2.0f * (quat.w * quat.z + quat.x * quat.y),
            1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z)))
    };
}

RouteObject::RouteObject(vsg::ref_ptr<vsg::PagedLOD> paged_lod,
    const std::string& label, vsg::vec3 translation, vsg::vec3 rotation_deg)
{
    assert(paged_lod);

    this->label = label;
    this->translation = translation;
    this->rotation_deg = rotation_deg;

    update_matrix();

    paged_lod_switch = SingleSwitch::create(
        vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    outline_switch = SingleSwitch::create(vsg::MASK_OFF,
        Outline::create(paged_lod));

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

vsg::vec3 RouteObject::get_initial_translation() const
{
    return initial_translation;
}

vsg::vec3 RouteObject::get_initial_rotation_deg() const
{
    return initial_rotation_deg;
}

vsg::vec3 RouteObject::get_initial_scale() const
{
    return initial_scale;
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

RouteObjects& RouteObject::get_selected_objects()
{
    return s_selected_objects;
}

RouteObjects& RouteObject::get_hidden_objects()
{
    return s_hidden_objects;
}

void RouteObject::set_observer_viewer(
    vsg::observer_ptr<vsg::Viewer> observer_viewer)
{
    s_observer_viewer = observer_viewer;
}

void RouteObject::set_translation(vsg::vec3 translation, bool update_matrix)
{
    this->translation = translation;

    if (update_matrix)
    {
        this->update_matrix();
    }
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

void RouteObject::move(vsg::vec3 translation, bool update_matrix)
{
    this->translation += translation;

    if (update_matrix)
    {
        this->update_matrix();
    }
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

void RouteObject::rotate_relative_to_point(vsg::vec3 point,
    vsg::vec3 rotation_deg)
{
    const vsg::vec3 rotation_rad = {
        vsg::radians(rotation_deg.x),
        vsg::radians(rotation_deg.y),
        vsg::radians(rotation_deg.z)
    };

    const vsg::mat4 rotate_x = vsg::rotate(rotation_rad.x, AXIS_X_POSITIVE);
    const vsg::mat4 rotate_y = vsg::rotate(rotation_rad.y, AXIS_Y_POSITIVE);
    const vsg::mat4 rotate_z = vsg::rotate(rotation_rad.z, AXIS_Z_POSITIVE);

    this->matrix = vsg::translate(point) * rotate_z * rotate_y * rotate_x *
        vsg::translate(-point) * static_cast<vsg::mat4>(initial_matrix);
}

void RouteObject::scale_relative_to_point(vsg::vec3 point, vsg::vec3 scale,
    bool update_matrix)
{
    // TODO

    if (update_matrix)
    {
        this->update_matrix();
    }
}

void RouteObject::hide()
{
    paged_lod_switch->mask = vsg::MASK_OFF;
    outline_switch->mask = vsg::MASK_OFF;

    is_hidden = true;

    s_hidden_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::show()
{
    paged_lod_switch->mask = MASK_SCENE | MASK_CLICKABLE;

    is_hidden = false;

    return s_hidden_objects.erase(std::find(s_hidden_objects.cbegin(),
        s_hidden_objects.cend(), this));
}

void RouteObject::select()
{
    const auto outline = outline_switch->node.cast<Outline>();
    outline->load(s_observer_viewer);

    outline_switch->mask = MASK_GUI2;

    is_selected = true;

    s_selected_objects.emplace_back(this);
}

RouteObjectsIterator RouteObject::deselect()
{
    outline_switch->mask = vsg::MASK_OFF;

    is_selected = false;

    return s_selected_objects.erase(std::find(s_selected_objects.cbegin(),
        s_selected_objects.cend(), this));
}

void RouteObject::save_translation()
{
    initial_translation = translation;
}

void RouteObject::save_rotation()
{
    initial_rotation_deg = rotation_deg;
}

void RouteObject::save_scale()
{
    initial_scale = scale_value;
}

void RouteObject::save_matrix()
{
    initial_matrix = this->matrix;
}

void RouteObject::update_matrix()
{
    const vsg::vec3 rotation_rad = {
        vsg::radians(rotation_deg.x),
        vsg::radians(rotation_deg.y),
        vsg::radians(rotation_deg.z)
    };

    const vsg::mat4 rotate_x = vsg::rotate(rotation_rad.x, AXIS_X_POSITIVE);
    const vsg::mat4 rotate_y = vsg::rotate(rotation_rad.y, AXIS_Y_POSITIVE);
    const vsg::mat4 rotate_z = vsg::rotate(rotation_rad.z, AXIS_Z_POSITIVE);

    this->matrix = vsg::translate(translation) * rotate_z *
        rotate_y * rotate_x * vsg::scale(scale_value);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds = static_cast<vsg::box>(compute_bounds.bounds);
}
