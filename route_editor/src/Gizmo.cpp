#include "Gizmo.h"

#include "Camera.h"
#include "EditorContext.h"
#include "Mask.h"
#include "Mouse.h"
#include "RouteObject.h"
#include "SingleSwitch.h"
#include "commands/CommandList.h"
#include "commands/TranslateObjects.h"
#include "editor_math.h"
#include "settings/GizmoSettings.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <cmath>
#include <cstdlib>
#include <limits>

static void rotate_geometry_info(
    vsg::GeometryInfo& geometry_info,
    vsg::vec3 direction
)
{
    constexpr vsg::vec3 Z_AXIS = {0.0f, 0.0f, 1.0f};
    if (vsg::length(vsg::cross(Z_AXIS, direction)) > 0.001f)
    {
        const vsg::vec3 axis = vsg::cross(Z_AXIS, direction);
        const float angle = std::acos(vsg::dot(Z_AXIS, direction));
        geometry_info.transform = vsg::rotate(angle, axis);
    }
}

Gizmo::Gizmo(
    EditorContext& context,
    const gizmo_settings_t& gizmo_settings,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list,
    const vsg::ref_ptr<Mouse>& mouse,
    const VkExtent2D& window_extent
)
    : context_(context)
    , gizmo_settings(gizmo_settings)
    , camera(camera)
    , command_list(command_list)
    , mouse(mouse)
    , window_extent(window_extent)
{
    builder_.shaderSet = vsg::createFlatShadedShaderSet();

    const float plane_width = 1.0e6f;
    const float line_thickness = 0.01f;

    vsg::StateInfo state_info;
    state_info.two_sided = true;
    state_info.blending = true;

    const auto create_arrow = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        float thickness = gizmo_settings.arrow_thickness;
        const float length = gizmo_settings.arrow_length;
        const float opacity = gizmo_settings.opacity;

        vsg::box box = {
            vsg::vec3(-thickness, -thickness, 0.0f),
            vsg::vec3( thickness,  thickness, length)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        const auto cylinder = builder_.createCylinder(geometry_info, state_info);

        thickness *= 3.0f;

        box.min = {-thickness, -thickness, length};
        box.max = { thickness,  thickness, length + thickness * 5.0f};

        geometry_info.set(box);

        const auto cone = builder_.createCone(geometry_info, state_info);

        const auto arrow = vsg::Group::create();
        arrow->addChild(cylinder);
        arrow->addChild(cone);

        return arrow;
    };

    const auto create_plane = [&](vsg::vec3 normal) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;

        const vsg::box box = {
            vsg::vec3(-width, -width, 0.0f),
            vsg::vec3( width,  width, 0.0f)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, normal);

        return builder_.createQuad(geometry_info, state_info);
    };

    const auto create_line = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;
        const float thickness = line_thickness;
        const float opacity = gizmo_settings.opacity;

        const vsg::box box = {
            vsg::vec3(-thickness, -thickness, -width),
            vsg::vec3( thickness,  thickness,  width)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        return builder_.createCylinder(geometry_info, state_info);
    };

    matrix_transform_ = vsg::MatrixTransform::create();

    constexpr vsg::vec3 arrow_directions[3] = {
        vsg::vec3(1.0f, 0.0f, 0.0f),
        vsg::vec3(0.0f, 1.0f, 0.0f),
        vsg::vec3(0.0f, 0.0f, 1.0f)
    };

    const vsg::vec3 arrow_colors[3] = {
        gizmo_settings.arrow_x_color,
        gizmo_settings.arrow_y_color,
        gizmo_settings.arrow_z_color
    };

    for (int i = 0; i < 3; ++i)
    {
        arrows[i] = create_arrow(arrow_directions[i], arrow_colors[i]);
        plane_switches[i] = SingleSwitch::create(vsg::MASK_OFF,
            create_plane(arrow_directions[i]));
        line_switches[i] = SingleSwitch::create(vsg::MASK_OFF, create_line(
            arrow_directions[i], arrow_colors[i]));
        matrix_transform_->addChild(arrows[i]);
        matrix_transform_->addChild(plane_switches[i]);
        matrix_transform_->addChild(line_switches[i]);
    }

    this->node = matrix_transform_;

    update_visibility();
}

bool Gizmo::handle_intersections()
{
    constexpr vsg::dvec3 arrow_directions[3] = {
        vsg::dvec3(1.0, 0.0, 0.0),
        vsg::dvec3(0.0, 1.0, 0.0),
        vsg::dvec3(0.0, 0.0, 1.0)
    };

    double arrow_dots[3];
    for (int i = 0; i < 3; ++i)
    {
        arrow_dots[i] = std::abs(vsg::dot(camera->get_front(), arrow_directions[i]));
    }

    // Положение мыши нормализованное [-1.0; 1.0]
    double norm_mouse_x, norm_mouse_y;
    normalize_mouse_coordinates(mouse->get_x(), mouse->get_y(), window_extent,
        norm_mouse_x, norm_mouse_y);

    const vsg::dmat4& inv_proj_mat = camera->get_inverse_projection_matrix();
    const vsg::dmat4& inv_view_mat = camera->get_inverse_view_matrix();

    vsg::dvec3 ray_origin, ray_end;
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 0.0,
        inv_view_mat, inv_proj_mat, ray_origin);
    calculate_mouse_world_coordinates(norm_mouse_x, norm_mouse_y, 1.0,
        inv_view_mat, inv_proj_mat, ray_end);

    const vsg::dvec3 ray_dir = ray_end - ray_origin;

    const double R_cyl = gizmo_settings.arrow_thickness * scale_;
    const double R_cone = gizmo_settings.arrow_thickness * 3.0 * scale_;
    const double H_cyl = gizmo_settings.arrow_length * scale_;
    const double H_cone = gizmo_settings.arrow_thickness * 15.0 * scale_;

    constexpr double EPSILON = 1e-9;
    double closest_t = std::numeric_limits<double>::max();
    int hit_arrow_index = -1;
    vsg::dvec3 hit_point;

    for (int axis = 0; axis < 3; ++axis)
    {
        const vsg::dvec3 axis_start = curr_pos_;
        const vsg::dvec3 axis_dir = arrow_directions[axis];

        vsg::dvec3 local_origin, local_dir;

        if (axis == 0)
        {
            local_origin = ray_origin - axis_start;
            local_dir = ray_dir;
        }
        else if (axis == 1)
        {
            local_origin = vsg::dvec3(
                ray_origin.y - axis_start.y,
                ray_origin.x - axis_start.x,
                ray_origin.z - axis_start.z
            );
            local_dir = vsg::dvec3(ray_dir.y, ray_dir.x, ray_dir.z);
        }
        else
        {
            local_origin = vsg::dvec3(
                ray_origin.z - axis_start.z,
                ray_origin.y - axis_start.y,
                ray_origin.x - axis_start.x
            );
            local_dir = vsg::dvec3(ray_dir.z, ray_dir.y, ray_dir.x);
        }

        const double A_cyl = local_dir.y * local_dir.y + local_dir.z * local_dir.z;

        if (std::abs(A_cyl) > EPSILON)
        {
            const double B_cyl = 2.0 * (local_origin.y * local_dir.y + local_origin.z * local_dir.z);
            const double C_cyl = local_origin.y * local_origin.y + local_origin.z * local_origin.z - R_cyl * R_cyl;

            double ts[2];
            if (solve_quadratic_equation(A_cyl, B_cyl, C_cyl, ts[0], ts[1]))
            {
                for (double t : ts)
                {
                    if (t >= 0.0 && t < closest_t)
                    {
                        const double x_hit = local_origin.x + t * local_dir.x;
                        if (x_hit >= 0.0 && x_hit <= H_cyl)
                        {
                            closest_t = t;
                            hit_arrow_index = axis;
                            hit_point = ray_origin + ray_dir * t;
                        }
                    }
                }
            }
        }

        if (std::abs(local_dir.x) > EPSILON)
        {
            double t_cap = -local_origin.x / local_dir.x;
            if (t_cap >= 0.0 && t_cap < closest_t)
            {
                const double y_hit = local_origin.y + t_cap * local_dir.y;
                const double z_hit = local_origin.z + t_cap * local_dir.z;
                if (y_hit * y_hit + z_hit * z_hit <= R_cyl * R_cyl)
                {
                    closest_t = t_cap;
                    hit_arrow_index = axis;
                    hit_point = ray_origin + ray_dir * t_cap;
                }
            }

            t_cap = (H_cyl - local_origin.x) / local_dir.x;
            if (t_cap >= 0.0 && t_cap < closest_t)
            {
                const double y_hit = local_origin.y + t_cap * local_dir.y;
                const double z_hit = local_origin.z + t_cap * local_dir.z;
                if (y_hit * y_hit + z_hit * z_hit <= R_cyl * R_cyl)
                {
                    closest_t = t_cap;
                    hit_arrow_index = axis;
                    hit_point = ray_origin + ray_dir * t_cap;
                }
            }
        }

        const vsg::dvec3 cone_origin = local_origin - vsg::dvec3(H_cyl, 0.0, 0.0);

        const double A_cone = local_dir.y * local_dir.y + local_dir.z * local_dir.z -
                             (R_cone / H_cone) * (R_cone / H_cone) * local_dir.x * local_dir.x;
        const double B_cone = 2.0 * (cone_origin.y * local_dir.y + cone_origin.z * local_dir.z -
                             (R_cone / H_cone) * (R_cone / H_cone) * cone_origin.x * local_dir.x);
        const double C_cone = cone_origin.y * cone_origin.y + cone_origin.z * cone_origin.z -
                             (R_cone / H_cone) * (R_cone / H_cone) * cone_origin.x * cone_origin.x;

        if (std::abs(A_cone) > EPSILON)
        {
            double ts[2];
            if (solve_quadratic_equation(A_cone, B_cone, C_cone, ts[0], ts[1]))
            {
                for (double t : ts)
                {
                    if (t >= 0.0 && t < closest_t)
                    {
                        const double x_hit = cone_origin.x + t * local_dir.x;
                        if (x_hit >= 0.0 && x_hit <= H_cone)
                        {
                            closest_t = t;
                            hit_arrow_index = axis;
                            hit_point = ray_origin + ray_dir * t;
                        }
                    }
                }
            }
        }
        else if (std::abs(A_cone) <= EPSILON && std::abs(B_cone) > EPSILON)
        {
            const double t = -C_cone / B_cone;
            if (t >= 0.0 && t < closest_t)
            {
                const double x_hit = cone_origin.x + t * local_dir.x;
                if (x_hit >= 0.0 && x_hit <= H_cone)
                {
                    closest_t = t;
                    hit_arrow_index = axis;
                    hit_point = ray_origin + ray_dir * t;
                }
            }
        }

        if (std::abs(local_dir.x) > EPSILON)
        {
            const double t_base = (H_cyl - local_origin.x) / local_dir.x;
            if (t_base >= 0.0 && t_base < closest_t)
            {
                const double y_hit = local_origin.y + t_base * local_dir.y;
                const double z_hit = local_origin.z + t_base * local_dir.z;
                if (y_hit * y_hit + z_hit * z_hit <= R_cone * R_cone)
                {
                    closest_t = t_base;
                    hit_arrow_index = axis;
                    hit_point = ray_origin + ray_dir * t_base;
                }
            }
        }
    }

    if (hit_arrow_index >= 0)
    {
        active_arrow_index = hit_arrow_index;

        if (hit_arrow_index == 0)
        {
            active_plain_index = (arrow_dots[1] > arrow_dots[2]) ? 1 : 2;
        }
        else if (hit_arrow_index == 1)
        {
            active_plain_index = (arrow_dots[2] > arrow_dots[0]) ? 2 : 0;
        }
        else
        {
            active_plain_index = (arrow_dots[0] > arrow_dots[1]) ? 0 : 1;
        }

        vsg::dvec3 click_pos = curr_pos_;
        click_pos[active_arrow_index] = hit_point[active_arrow_index];

        prev_intersect_pos_ = click_pos;
        total_translation_.set(0.0, 0.0, 0.0);

        plane_switches[active_plain_index]->mask = MASK_CLICKABLE;
        line_switches[active_arrow_index]->mask = MASK_GUI1;

        for (const auto& object : context_.selected_objects)
        {
            object->save_matrix();
        }

        return true;
    }

    return false;
}

void Gizmo::apply(const vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled || active_arrow_index < 0)
    {
        return;
    }

    command_list.push(new TranslateObjects(
        context_, context_.selected_objects, total_translation_), false);

    plane_switches[active_plain_index]->mask = vsg::MASK_OFF;
    line_switches[active_arrow_index]->mask = vsg::MASK_OFF;

    active_arrow_index = -1;
    active_plain_index = -1;
}

void Gizmo::apply(const vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || active_plain_index < 0)
    {
        return;
    }

    const auto intersector = vsg::LineSegmentIntersector::create(*camera,
        moveEvent.x, moveEvent.y);
    if (!intersector)
    {
        return;
    }
    intersector->traversalMask = MASK_CLICKABLE;

    this->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        return;
    }

    std::sort(intersections.begin(), intersections.end(),
        [](const auto& lhs, const auto& rhs) -> bool {
            return (lhs->ratio) < (rhs->ratio);
        }
    );

    const auto& intersection = intersections.front();

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if ((node != plane_switches[active_plain_index]) || (active_arrow_index < 0))
        {
            continue;
        }

        vsg::dvec3 translation = {0.0, 0.0, 0.0};

        translation[active_arrow_index] =
            world_intersection[active_arrow_index] -
            prev_intersect_pos_[active_arrow_index];

        prev_intersect_pos_[active_arrow_index] =
            world_intersection[active_arrow_index];

        total_translation_ += translation;

        for (const auto& object : context_.selected_objects)
        {
            object->move(translation);
        }

        return;
    }
}

const vsg::dvec3& Gizmo::get_curr_pos() const
{
    return curr_pos_;
}

void Gizmo::update_visibility()
{
    this->mask = context_.selected_objects.empty()
        ? vsg::MASK_OFF
        : MASK_GUI1 | MASK_CLICKABLE;

    const vsg::dvec3& camera_pos = camera->get_look_at()->eye;
    const double fov_rad = vsg::radians(camera->get_perspective()->fieldOfViewY);

    const double distance_to_camera = vsg::length(curr_pos_ - camera_pos);
    const double tan_half_fov = std::tan(fov_rad * 0.5);
    scale_ = distance_to_camera * tan_half_fov * 0.075;

    matrix_transform_->matrix = vsg::translate(curr_pos_) * vsg::scale(scale_);
}

static vsg::dvec3 calculate_position_pivot(
    const RouteObjects& selected_objects
)
{
    vsg::dvec3 pos = {0.0, 0.0, 0.0};
    for (const auto& object : selected_objects)
    {
        pos += object->get_translation();
    }
    pos /= selected_objects.size();
    return pos;
}

static vsg::dvec3 calculate_position_center(
    const RouteObjects& selected_objects
)
{
    vsg::dvec3 pos = {0.0, 0.0, 0.0};
    for (const auto& object : selected_objects)
    {
        const vsg::dbox& bounds = object->get_bounds();
        pos += (bounds.min + bounds.max) * 0.5;
    }
    pos /= selected_objects.size();
    return pos;
}

void Gizmo::update_position()
{
    curr_pos_ = gizmo_settings.to_center
        ? calculate_position_center(context_.selected_objects)
        : calculate_position_pivot(context_.selected_objects);

    matrix_transform_->matrix = vsg::translate(curr_pos_) * vsg::scale(scale_);
}
