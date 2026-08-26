#include "editor_math.h"

#include <vulkan/vulkan_core.h>

#include <cmath>

void normalize_mouse_coordinates(int x, int y, VkExtent2D extent,
    double& norm_x, double& norm_y)
{
    norm_x = static_cast<double>(x) / extent.width * 2.0 - 1.0;
    norm_y = static_cast<double>(y) / extent.height * 2.0 - 1.0;
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
