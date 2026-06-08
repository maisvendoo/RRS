#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "settings/CameraSettings.h"
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

namespace vsg
{

class FrameEvent;
class MoveEvent;
class Node;
class ScrollWheelEvent;

}

class CameraHandler : public vsg::Inherit<vsg::Visitor, CameraHandler>
{
public:
    CameraHandler(
        const camera_settings_t& camera_settings,
        vsg::ref_ptr<vsg::Perspective>& perspective,
        vsg::ref_ptr<vsg::LookAt>& look_at,
        vsg::ref_ptr<vsg::Camera>& camera,
        VkExtent2D window_extent,
        vsg::ref_ptr<MouseHandler>& mouse_handler,
        vsg::ref_ptr<KeyboardHandler>& keyboard_handler,
        double& delta_time
    );

    virtual ~CameraHandler() = default;

    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;
    virtual void apply(vsg::FrameEvent& frame) override;

    const vsg::dvec3& get_front() const;
    const vsg::dvec3& get_right() const;
    const vsg::dvec3& get_up() const;

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
    vsg::ref_ptr<vsg::Perspective>& perspective;
    vsg::ref_ptr<vsg::LookAt>& look_at;
    vsg::ref_ptr<vsg::Camera>& camera;
    vsg::ref_ptr<MouseHandler>& mouse_handler;
    vsg::ref_ptr<KeyboardHandler>& keyboard_handler;
    double& delta_time;

    double yaw_deg_ = 0.0;
    double pitch_deg_ = 0.0;

    vsg::dvec3 front_;
    vsg::dvec3 right_;
    vsg::dvec3 up_;
};

#endif // CAMERA_HANDLER_H
