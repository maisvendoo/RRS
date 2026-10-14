#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

struct EditorContext;

namespace vsg
{

class LookAt;
class Orthographic;
class Perspective;
class ProjectionMatrix;

} // namespace vsg

class Camera : public vsg::Inherit<vsg::Camera, Camera>
{
public:
    Camera(EditorContext& editor_context);

    virtual ~Camera() = default;

    void swap_proj_mat();

    void update_move_dir();

    void handle_mouse_move();

    void handle_mouse_scroll();

    void update(double delta_time);

    const vsg::dvec3& get_front() const;

    const vsg::dvec3& get_right() const;

    const vsg::dvec3& get_up() const;

    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;

    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;

    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

    const vsg::dmat4& get_inv_proj_mat() const;

    const vsg::dmat4& get_inv_view_mat() const;

    void look_on(const vsg::dvec3& pos, double distance = 50.0);

private:
    void create_orthographic_projection(double aspect_ratio);

    void calc_front();

    void calc_right();

    void calc_up();

    void calc_inv_proj_mat();

    void calc_inv_view_mat();

private:
    EditorContext& editor_context;

    vsg::ref_ptr<vsg::ProjectionMatrix> another_proj_mat;
    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::Orthographic> orthographic;
    vsg::ref_ptr<vsg::LookAt> look_at;

    vsg::dmat4 inv_proj_mat;
    vsg::dmat4 inv_view_mat;

    double yaw_deg = 0.0;
    double pitch_deg = 0.0;

    vsg::dvec3 front;
    vsg::dvec3 right;
    vsg::dvec3 up;

    vsg::dvec3 move_dir = {0.0, 0.0, 0.0};
};

#endif // EDITOR_CAMERA_H
