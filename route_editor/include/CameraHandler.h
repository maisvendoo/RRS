#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

class KeyboardHandler;
class MouseHandler;
struct settings_t;

namespace vsg
{

class Camera;
class FrameEvent;
class LookAt;
class Perspective;

}

class CameraHandler : public vsg::Inherit<vsg::Visitor, CameraHandler>
{
public:
    using value_type = float;
    using vec2_type = vsg::t_vec2<value_type>;
    using vec3_type = vsg::t_vec3<value_type>;

public:
    CameraHandler(
        const settings_t& settings,
        const VkExtent2D& window_extent,
        vsg::ref_ptr<MouseHandler> mouse_handler,
        vsg::ref_ptr<KeyboardHandler> keyboard_handler
    );

    void apply(vsg::FrameEvent& frame) override;

    vsg::ref_ptr<vsg::Perspective> get_perspective() const;
    vsg::ref_ptr<vsg::LookAt> get_look_at() const;
    vsg::ref_ptr<vsg::Camera> get_camera() const;

    vec3_type get_front() const;
    vec3_type get_right() const;
    vec3_type get_up() const;

private:
    void calculate_front();
    void calculate_right();
    void calculate_up();

private:
    const settings_t& settings;
    vsg::ref_ptr<MouseHandler> mouse_handler;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;

    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::ref_ptr<vsg::Camera> camera;

    value_type yaw_deg = 0.0;
    value_type pitch_deg = 0.0;

    vec3_type front;
    vec3_type right;
    vec3_type up;
};

#endif // CAMERA_HANDLER_H
