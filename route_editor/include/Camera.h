#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>

class Keyboard;
class Mouse;
struct camera_settings_t;

namespace vsg
{

class LookAt;
class Orthographic;
class Perspective;
class ProjectionMatrix;

}

class Camera : public vsg::Inherit<vsg::Camera, Camera>
{
public:
    Camera(
        const camera_settings_t& camera_settings,
        const VkExtent2D& window_extent,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );

    virtual ~Camera() = default;

    void handle_key_press();

    void update_move_direction();

    void handle_mouse_move();

    void handle_mouse_scroll();

    void update(double delta_time);

    const vsg::dvec3& get_front() const;

    const vsg::dvec3& get_right() const;

    const vsg::dvec3& get_up() const;

    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;

    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;

    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

    const vsg::dmat4& get_inverse_projection_matrix() const;

    const vsg::dmat4& get_inverse_view_matrix() const;

private:
    void create_orthographic_projection(double window_width,
        double window_height, double aspect_ratio);

    void calculate_front();

    void calculate_right();

    void calculate_up();

    void calculate_inverse_projection_matrix();

    void calculate_inverse_view_matrix();

private:
    const camera_settings_t& camera_settings;
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;

    vsg::ref_ptr<vsg::ProjectionMatrix> another_projection_matrix;
    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::Orthographic> orthographic;
    vsg::ref_ptr<vsg::LookAt> look_at;

    vsg::dmat4 inverse_projection_matrix;
    vsg::dmat4 inverse_view_matrix;

    double yaw_degrees = 0.0;
    double pitch_degrees = 0.0;

    vsg::dvec3 front_;
    vsg::dvec3 right_;
    vsg::dvec3 up_;

    vsg::dvec3 move_direction = {0.0, 0.0, 0.0};
};

#endif // CAMERA_HANDLER_H
