#include "RouteObject.h"

#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>

static constexpr vsg::vec3 AXIS_X_POSITIVE = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Y_POSITIVE = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 AXIS_Z_POSITIVE = {0.0f, 0.0f, 1.0f};

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
}
