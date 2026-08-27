#include "editor_math.h"

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

#include <cmath>

void normalize_mouse_coordinates(int x, int y, VkExtent2D extent,
    double& norm_x, double& norm_y)
{
    norm_x = static_cast<double>(x) / extent.width * 2.0 - 1.0;
    norm_y = static_cast<double>(y) / extent.height * 2.0 - 1.0;
}

void calculate_mouse_world_coordinates(
    int x, int y, double z, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& out
)
{
    double norm_x, norm_y;
    normalize_mouse_coordinates(x, y, extent, norm_x, norm_y);
    calculate_mouse_world_coordinates(norm_x, norm_y, z,
        inv_view_mat, inv_proj_mat, out);
}

void calculate_mouse_world_coordinates(
    double norm_x, double norm_y, double z,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& out
)
{
    out = inv_view_mat * inv_proj_mat * vsg::dvec3(norm_x, norm_y, z);
}

bool solve_quadratic_equation(double a, double b, double c,
    double& x1, double& x2)
{
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

bool calculate_intersection_line_and_plane(
    vsg::dvec3 line_orig, vsg::dvec3 line_dir,
    vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
    vsg::dvec3& out
)
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

    const double t = -(vsg::dot(norm, orig) - vsg::dot(norm, point)) / denom;
    out = orig + dir * t;

    return true;
}

bool calculate_intersection_mouse_and_plane(
    int x, int y, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
    vsg::dvec3& out
)
{
    double norm_x, norm_y;
    normalize_mouse_coordinates(x, y, extent, norm_x, norm_y);

    vsg::dvec3 mouse_world1, mouse_world2;
    calculate_mouse_world_coordinates(norm_x, norm_y, 0.0, extent,
        inv_view_mat, inv_proj_mat, mouse_world1);
    calculate_mouse_world_coordinates(norm_x, norm_y, 1.0, extent,
        inv_view_mat, inv_proj_mat, mouse_world2);

    return calculate_intersection_line_and_plane(mouse_world1,
        mouse_world2 - mouse_world1, plane_point, plane_norm, out);
}
