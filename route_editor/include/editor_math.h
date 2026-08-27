#ifndef EDITOR_MATH_H
#define EDITOR_MATH_H

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

void normalize_mouse_coordinates(int x, int y, VkExtent2D extent,
    double& norm_x, double& norm_y);

void calculate_mouse_world_coordinates(
    int x, int y, double z, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& out
);

void calculate_mouse_world_coordinates(
    double norm_x, double norm_y, double z,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3& out
);

bool solve_quadratic_equation(double a, double b, double c,
    double& x1, double& x2);

bool calculate_intersection_line_and_plane(
    vsg::dvec3 line_orig, vsg::dvec3 line_dir,
    vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
    vsg::dvec3& out
);

bool calculate_intersection_mouse_and_plane(
    int x, int y, VkExtent2D extent,
    const vsg::dmat4& inv_view_mat, const vsg::dmat4& inv_proj_mat,
    vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
    vsg::dvec3& out
);

#endif // EDITOR_MATH_H
