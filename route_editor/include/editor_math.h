#ifndef EDITOR_MATH_H
#define EDITOR_MATH_H

struct VkExtent2D;

void normalize_mouse_coordinates(int x, int y, VkExtent2D extent,
    double& norm_x, double& norm_y);

bool solve_quadratic_equation(double a, double b, double c,
    double& x1, double& x2);

#endif // EDITOR_MATH_H
