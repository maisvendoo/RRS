#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <vsg/app/Camera.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

class Keyboard;
class Mouse;
struct camera_settings_t;

namespace vsg
{

class LookAt;
class Node;
class Orthographic;
class Perspective;

}

class Camera : public vsg::Inherit<vsg::Camera, Camera>
{
public:
    Camera(
        const camera_settings_t& camera_settings,
        VkExtent2D window_extent,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );

    virtual ~Camera() = default;

    void handle_key_press();

    void update_move_direction();

    void handle_mouse_move();

    void update(double delta_time);

    void handle_mouse_scroll();

    const vsg::dvec3& get_front() const;

    const vsg::dvec3& get_right() const;

    const vsg::dvec3& get_up() const;

    const vsg::ref_ptr<vsg::Perspective>& get_perspective() const;

    const vsg::ref_ptr<vsg::Orthographic>& get_orthographic() const;

    const vsg::ref_ptr<vsg::LookAt>& get_look_at() const;

    // Create plane perpedicular to camera normal and passing through
    // specified point to test for intersections
    vsg::ref_ptr<vsg::Node> create_front_plane(
        const vsg::dvec3& point,
        vsg::dvec3* up_out = nullptr
    ) const;

private:
    void calculate_front();

    void calculate_right();

    void calculate_up();

private:
    const camera_settings_t& camera_settings;
    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::Orthographic> orthographic;
    vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::ref_ptr<vsg::Camera> camera;
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;

    double yaw_deg_ = 0.0;
    double pitch_deg_ = 0.0;

    vsg::dvec3 front_;
    vsg::dvec3 right_;
    vsg::dvec3 up_;

    vsg::dvec3 move_direction = {0.0, 0.0, 0.0};
};

#endif // CAMERA_HANDLER_H
