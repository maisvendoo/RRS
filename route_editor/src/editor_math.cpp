#include "editor/editor_math.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>

void normalize_mouse_coordinates(int x, int y, VkExtent2D extent,
    double& norm_x, double& norm_y)
{
    norm_x = static_cast<double>(x) / extent.width * 2.0 - 1.0;
    norm_y = static_cast<double>(y) / extent.height * 2.0 - 1.0;
}

bool calculate_mouse_world_coordinates(double norm_x, double norm_y, double z,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& out)
{
    vsg::dvec4 clip = {norm_x, norm_y, z, 1.0};

    vsg::dvec4 world = inv_view_mat * inv_proj_mat * clip;
    if (std::abs(world.w) < 1.0e-6)
    {
        return false;
    }

    out = vsg::dvec3(world.x, world.y, world.z) / world.w;

    return true;
}

bool calculate_mouse_world_coordinates(int x, int y, double z,
    VkExtent2D extent, const vsg::dmat4& inv_view_mat,
    const vsg::dmat4& inv_proj_mat, vsg::dvec3& out)
{
    double norm_x, norm_y;
    normalize_mouse_coordinates(x, y, extent, norm_x, norm_y);
    return calculate_mouse_world_coordinates(norm_x, norm_y, z, inv_view_mat,
        inv_proj_mat, out);
}

bool calculate_mouse_ray(double norm_x, double norm_y,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& origin, vsg::dvec3& dir)
{
    vsg::dvec3 mouse_world1, mouse_world2;

    if (!calculate_mouse_world_coordinates(norm_x, norm_y, 0.0, inv_view_mat,
        inv_proj_mat, mouse_world1))
    {
        return false;
    }


    if (!calculate_mouse_world_coordinates(norm_x, norm_y, 1.0, inv_view_mat,
        inv_proj_mat, mouse_world2))
    {
        return false;
    }

    origin = mouse_world1;
    dir = vsg::normalize(mouse_world2 - mouse_world1);

    return true;
}

bool calculate_mouse_ray(int x, int y, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& origin, vsg::dvec3& dir)
{
    double norm_x, norm_y;
    normalize_mouse_coordinates(x, y, extent, norm_x, norm_y);
    return calculate_mouse_ray(norm_x, norm_y, inv_view_mat, inv_proj_mat,
        origin, dir);
}

bool solve_quadratic_equation(double a, double b, double c, double& x1,
    double& x2)
{
    if (std::abs(a) < 1.0e-6)
    {
        return false;
    }

    const double D = b * b - 4 * a * c;
    if (D < 0.0)
    {
        return false;
    }

    const double sqrt_D = std::sqrt(D);
    const double inv_2a = 1.0 / (2.0 * a);

    x1 = (-b + sqrt_D) * inv_2a;
    x2 = (-b - sqrt_D) * inv_2a;

    return true;
}

bool calculate_intersection_line_and_plane(vsg::dvec3 line_orig,
    vsg::dvec3 line_dir, vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
    vsg::dvec3& out)
{
    const vsg::dvec3 orig = line_orig;
    const vsg::dvec3 dir = line_dir;
    const vsg::dvec3 point = plane_point;
    const vsg::dvec3 norm = plane_norm;

    const double denom = vsg::dot(norm, dir);
    if (std::abs(denom) < 1.0e-6)
    {
        return false;
    }

    const double t = (vsg::dot(norm, point) - vsg::dot(norm, orig)) / denom;
    out = orig + dir * t;

    return true;
}

bool calculate_intersection_mouse_and_plane(int x, int y, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3 plane_point, vsg::dvec3 plane_norm, vsg::dvec3& out)
{
    vsg::dvec3 line_orig, line_dir;
    if (!calculate_mouse_ray(x, y, extent, inv_view_mat, inv_proj_mat,
        line_orig, line_dir))
    {
        return false;
    }

    return calculate_intersection_line_and_plane(line_orig, line_dir,
        plane_point, plane_norm, out);
}

bool calculate_closest_intersection_line_and_cylinder(int axis_index,
    vsg::dvec3 line_orig, vsg::dvec3 line_dir, vsg::dvec3 cylinder_base_center,
    double cylinder_radius, double cylinder_height, vsg::dvec3& out)
{
    if (axis_index < 0 || axis_index > 2)
    {
        return false;
    }

    const int axis1 = (axis_index + 1) % 3;
    const int axis2 = (axis_index + 2) % 3;

    const vsg::dvec3 orig = line_orig;
    const vsg::dvec3 dir = line_dir;
    const vsg::dvec3 center = cylinder_base_center;
    const double R = cylinder_radius;
    const double H = cylinder_height;
    const double V1 = orig[axis1] - center[axis1];
    const double V2 = orig[axis2] - center[axis2];

    const double A = dir[axis1] * dir[axis1] + dir[axis2] * dir[axis2];
    const double B = 2.0 * (dir[axis1] * V1 + dir[axis2] * V2);
    const double C = V1 * V1 + V2 * V2 - R * R;

    double t1, t2;
    if (!solve_quadratic_equation(A, B, C, t1, t2))
    {
        return false;
    }

    const vsg::dvec3 p1 = orig + dir * t1;
    const vsg::dvec3 p2 = orig + dir * t2;

    if (p1[axis_index] < center[axis_index] ||
        p1[axis_index] > center[axis_index] + H)
    {
        t1 = -1.0;
    }

    if (p2[axis_index] < center[axis_index] ||
        p2[axis_index] > center[axis_index] + H)
    {
        t2 = -1.0;
    }

    if (t1 < 0.0 && t2 < 0.0)
    {
        return false;
    }

    double t = std::min(t1, t2);
    t = (t < 0.0) ? std::max(t1, t2) : t;

    out = orig + dir * t;

    return true;
}

bool calculate_closest_intersection_mouse_and_cylinder(int axis_index, int x,
    int y, VkExtent2D extent, const vsg::dmat4& inv_view_mat,
    const vsg::dmat4& inv_proj_mat, vsg::dvec3 cylinder_base_center,
    double cylinder_radius, double cylinder_height, vsg::dvec3& out)
{
    vsg::dvec3 line_orig, line_dir;
    if (!calculate_mouse_ray(x, y, extent, inv_view_mat, inv_proj_mat,
        line_orig, line_dir))
    {
        return false;
    }

    return calculate_closest_intersection_line_and_cylinder(axis_index,
        line_orig, line_dir, cylinder_base_center, cylinder_radius,
        cylinder_height, out);
}

bool calculate_closest_intersection_line_and_cone(int axis_index,
    vsg::dvec3 line_orig, vsg::dvec3 line_dir, vsg::dvec3 cone_base_center,
    double cone_radius, double cone_height, vsg::dvec3& out)
{
    if (axis_index < 0 || axis_index > 2)
    {
        return false;
    }

    const int axis1 = (axis_index + 1) % 3;
    const int axis2 = (axis_index + 2) % 3;

    const vsg::dvec3 orig = line_orig;
    const vsg::dvec3 dir = line_dir;
    const vsg::dvec3 center = cone_base_center;
    const double R = cone_radius;
    const double H = cone_height;
    const double U = (R * R) / (H * H);
    const double V0 = center[axis_index] + H - orig[axis_index];
    const double V1 = orig[axis1] - center[axis1];
    const double V2 = orig[axis2] - center[axis2];

    const double A = dir[axis1] * dir[axis1] + dir[axis2] * dir[axis2] -
        U * dir[axis_index] * dir[axis_index];
    const double B = 2.0 * (dir[axis1] * V1 + dir[axis2] * V2 +
        U * dir[axis_index] * V0);
    const double C = V1 * V1 + V2 * V2 - U * V0 * V0;

    double t1, t2;
    if (!solve_quadratic_equation(A, B, C, t1, t2))
    {
        return false;
    }

    const vsg::dvec3 p1 = orig + dir * t1;
    const vsg::dvec3 p2 = orig + dir * t2;

    if (p1[axis_index] < center[axis_index] ||
        p1[axis_index] > center[axis_index] + H)
    {
        t1 = -1.0;
    }

    if (p2[axis_index] < center[axis_index] ||
        p2[axis_index] > center[axis_index] + H)
    {
        t2 = -1.0;
    }

    if (t1 < 0.0 && t2 < 0.0)
    {
        return false;
    }

    double t = std::min(t1, t2);
    t = (t < 0.0) ? std::max(t1, t2) : t;

    out = orig + dir * t;

    return true;
}

bool calculate_closest_intersection_mouse_and_cone(int axis_index, int x,
    int y, VkExtent2D extent, const vsg::dmat4& inv_view_mat,
    const vsg::dmat4& inv_proj_mat, vsg::dvec3 cone_base_center,
    double cone_radius, double cone_height, vsg::dvec3& out)
{
    vsg::dvec3 line_orig, line_dir;
    if (!calculate_mouse_ray(x, y, extent, inv_view_mat, inv_proj_mat,
        line_orig, line_dir))
    {
        return false;
    }

    return calculate_closest_intersection_line_and_cone(axis_index, line_orig,
        line_dir, cone_base_center, cone_radius, cone_height, out);
}
