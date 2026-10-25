#ifndef EDITOR_EDITOR_MATH_H
#define EDITOR_EDITOR_MATH_H

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

void norm_mouse_coords(int x, int y, VkExtent2D extent, double& norm_x,
                       double& norm_y);

bool calc_mouse_world_coords(double norm_x, double norm_y, double z,
                             const vsg::dmat4& inv_view,
                             const vsg::dmat4& inv_proj, vsg::dvec3& out);

bool calc_mouse_world_coords(int x, int y, double z, VkExtent2D extent,
                             const vsg::dmat4& inv_view,
                             const vsg::dmat4& inv_proj, vsg::dvec3& out);

bool calc_mouse_ray(double norm_x, double norm_y, const vsg::dmat4& inv_view,
                    const vsg::dmat4& inv_proj, vsg::dvec3& origin,
                    vsg::dvec3& dir);

bool calc_mouse_ray(int x, int y, VkExtent2D extent, const vsg::dmat4& inv_view,
                    const vsg::dmat4& inv_proj, vsg::dvec3& origin,
                    vsg::dvec3& dir);

bool solve_quad_eq(double a, double b, double c, double& x1, double& x2);

bool calc_intersection_line_plane(vsg::dvec3 line_orig, vsg::dvec3 line_dir,
                                  vsg::dvec3 plane_point, vsg::dvec3 plane_norm,
                                  vsg::dvec3& out);

bool calc_intersection_mouse_plane(int x, int y, VkExtent2D extent,
                                   const vsg::dmat4& inv_view,
                                   const vsg::dmat4& inv_proj,
                                   vsg::dvec3 plane_point,
                                   vsg::dvec3 plane_norm, vsg::dvec3& out);

bool calc_near_intersection_line_cylinder(int axis_index, vsg::dvec3 line_orig,
                                          vsg::dvec3 line_dir,
                                          vsg::dvec3 cyl_base_center,
                                          double cyl_radius, double cyl_height,
                                          vsg::dvec3& out);

bool calc_near_intersection_mouse_cylinder(
    int axis_index, int x, int y, VkExtent2D extent, const vsg::dmat4& inv_view,
    const vsg::dmat4& inv_proj, vsg::dvec3 cyl_base_center, double cyl_radius,
    double cyl_height, vsg::dvec3& out);

bool calc_near_intersection_line_cone(int axis_index, vsg::dvec3 line_orig,
                                      vsg::dvec3 line_dir,
                                      vsg::dvec3 cone_base_center,
                                      double cone_radius, double cone_height,
                                      vsg::dvec3& out);

bool calc_near_intersection_mouse_cone(
    int axis_index, int x, int y, VkExtent2D extent, const vsg::dmat4& inv_view,
    const vsg::dmat4& inv_proj, vsg::dvec3 cone_base_center, double cone_radius,
    double cone_height, vsg::dvec3& out);

#endif // EDITOR_EDITOR_MATH_H
