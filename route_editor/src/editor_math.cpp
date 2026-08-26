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
