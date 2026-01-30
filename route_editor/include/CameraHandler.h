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
    CameraHandler(
        const settings_t& settings,
        const VkExtent2D& window_extent,
        vsg::ref_ptr<MouseHandler> mouse_handler,
        vsg::ref_ptr<KeyboardHandler> keyboard_handler,
        double initial_height
    );

    void apply(vsg::FrameEvent& frame) override;

    vsg::ref_ptr<vsg::Perspective> get_perspective() const;
    vsg::ref_ptr<vsg::LookAt> get_look_at() const;
    vsg::ref_ptr<vsg::Camera> get_camera() const;

    vsg::dvec3 get_front() const;
    vsg::dvec3 get_right() const;

private:
    const settings_t& settings;
    vsg::ref_ptr<MouseHandler> mouse_handler;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;

    vsg::ref_ptr<vsg::Perspective> perspective;
    vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::ref_ptr<vsg::Camera> camera;

    double yaw_deg = 0.0;
    double pitch_deg = 0.0;
    vsg::dvec3 front;
    vsg::dvec3 right;
};

#endif // CAMERA_HANDLER_H
