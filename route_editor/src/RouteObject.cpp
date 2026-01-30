#include "RouteObject.h"

#include "Mask.h"
#include "Outline.h"
#include "SwitchGroup.h"

#include <vsg/core/Mask.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>

#include <cassert>
#include <string>
#include <vsg/utils/ComputeBounds.h>

static constexpr vsg::vec3 AXIS_X_POSITIVE = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Y_POSITIVE = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Z_POSITIVE = {0.0f, 0.0f, 1.0f};

RouteObject::RouteObject(vsg::ref_ptr<vsg::PagedLOD> paged_lod,
    const std::string& label, vsg::vec3 translation, vsg::vec3 rotation_deg)
{
    assert(paged_lod);

    this->label = label;
    this->translation = translation;
    this->rotation_deg = rotation_deg;

    update_matrix();

    switch_group = SwitchGroup::create();
    switch_group->addChild(vsg::Mask{MASK_SCENE | MASK_CLICKABLE}, paged_lod);

    this->addChild(switch_group);

    this->paged_lod = paged_lod;
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
    return scale;
}

const vsg::box& RouteObject::get_bounds() const
{
    return bounds;
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
    this->scale = scale;

    if (update_matrix)
    {
        this->update_matrix();
    }
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
        rotate_y * rotate_x * vsg::scale(scale);

    update_bounds();
}

void RouteObject::update_bounds()
{
    vsg::ComputeBounds compute_bounds;
    compute_bounds.useNodeBounds = false;
    this->accept(compute_bounds);
    bounds = static_cast<vsg::box>(compute_bounds.bounds);
}

vsg::ref_ptr<SwitchGroup> RouteObject::get_switch_group() const
{
    return switch_group;
}

vsg::ref_ptr<vsg::PagedLOD> RouteObject::get_paged_lod() const
{
    return paged_lod;
}
