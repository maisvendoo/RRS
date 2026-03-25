#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

struct EditorContext;
class RouteObject;
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
    ObjectSelector(EditorContext& context);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;

private:
    void select_object(RouteObject* object);

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

    State state = State::INITIAL;

    EditorContext& context;

    vsg::vec3 prev_intersect_pos;
    vsg::vec3 total_translation;
    float total_rotation_rad;
    vsg::vec3 total_scale;
    vsg::ref_ptr<SingleSwitch> front_plane_switch;
    vsg::vec3 front_plane_up;
};

#endif // OBJECT_SELECTOR_H
