#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

#include <memory>

class CameraNavigationState;
class GizmoRotateState;
class GizmoScaleState;
class GizmoTranslateState;
class InitialState;
class Keyboard;
class KeyboardRotateState;
class KeyboardScaleState;
class KeyboardTranslateState;
class SelectRouteState;
class State;

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(Keyboard* keyboard);
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

private:
    Keyboard* const keyboard_;

    std::unique_ptr<State>* state_;
    std::unique_ptr<State> select_route_state_;
    std::unique_ptr<State> initial_state_;
    std::unique_ptr<State> camera_navigation_state_;
    std::unique_ptr<State> keyboard_translate_state_;
    std::unique_ptr<State> keyboard_rotate_state_;
    std::unique_ptr<State> keyboard_scale_state_;
    std::unique_ptr<State> gizmo_translate_state_;
    std::unique_ptr<State> gizmo_rotate_state_;
    std::unique_ptr<State> gizmo_scale_state_;
};

#endif // EVENT_HANDLER_H
