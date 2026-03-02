#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <vulkan/vulkan_core.h>

struct EditorContext;

namespace vsg
{

class FrameEvent;
class Node;

}

class CameraHandler : public vsg::Inherit<vsg::Visitor, CameraHandler>
{
public:
    using value_type = float;
    using vec3_type = vsg::t_vec3<value_type>;

public:
    CameraHandler(EditorContext& context);

    void apply(vsg::FrameEvent& frame) override;

    vec3_type get_front() const;
    vec3_type get_right() const;
    vec3_type get_up() const;

    vsg::ref_ptr<vsg::Node> create_front_plane(vec3_type point,
        vec3_type* up_out = nullptr) const;

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
