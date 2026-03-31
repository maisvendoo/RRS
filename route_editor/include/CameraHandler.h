#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

struct EditorContext;

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
    using value_type = float;
    using vec3_type = vsg::t_vec3<value_type>;

public:
    explicit CameraHandler(EditorContext& context);
    virtual ~CameraHandler() = default;

    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;
    virtual void apply(vsg::FrameEvent& frame) override;

    const vec3_type& get_front() const;
    const vec3_type& get_right() const;
    const vec3_type& get_up() const;

    // Create plane perpedicular to camera normal and passing through
    // specified point to test for intersections
    vsg::ref_ptr<vsg::Node> create_front_plane(
        const vec3_type& point,
        vec3_type* up_out = nullptr
    ) const;

private:
    void calculate_front();
    void calculate_right();
    void calculate_up();

private:
    EditorContext& context;

    value_type yaw_deg = 0.0;
    value_type pitch_deg = 0.0;

    vec3_type front;
    vec3_type right;
    vec3_type up;
};

#endif // CAMERA_HANDLER_H
