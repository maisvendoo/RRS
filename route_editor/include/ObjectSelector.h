#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vulkan/vulkan_core.h>

class Camera;
class CommandList;
struct EditorContext;
class Gizmo;
class Keyboard;
class Mouse;
class Route;
class RouteObject;
class SceneGraph;
class SingleSwitch;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class KeyPressEvent;
class MoveEvent;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        EditorContext& context,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        const vsg::ref_ptr<Camera>& camera,
        CommandList& command_list,
        const vsg::ref_ptr<SceneGraph>& scene_graph,
        const vsg::ref_ptr<Route>& route,
        const VkExtent2D& window_extent,
        const vsg::ref_ptr<Gizmo>& gizmo
    );

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;

private:
    void select_object(vsg::ref_ptr<RouteObject> object);

    void confirm_keyboard_transformation();
    void cancel_keyboard_transformation();

private:
    enum class State
    {
        INITIAL,
        KEYBOARD_GRAB,
        KEYBOARD_ROTATE,
        KEYBOARD_SCALE
    };

    State state_ = State::INITIAL;

    EditorContext& context_;
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<Keyboard>& keyboard;
    const vsg::ref_ptr<Camera>& camera;
    CommandList& command_list;
    const vsg::ref_ptr<SceneGraph>& scene_graph;
    const vsg::ref_ptr<Route>& route;
    const VkExtent2D& window_extent;
    const vsg::ref_ptr<Gizmo>& gizmo;

    vsg::dvec3 prev_intersect_pos_;
    vsg::dvec3 total_translation_;
    double total_rotation_rad_;
    vsg::dvec3 total_scale_;
};

#endif // OBJECT_SELECTOR_H
