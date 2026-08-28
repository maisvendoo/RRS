#ifndef NAVIGATION_STATE_H
#define NAVIGATION_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Camera;
class Keyboard;
class Mouse;
class StateManager;

class NavigationState : public State
{
public:
    NavigationState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager,
        const vsg::ref_ptr<Camera>& camera
    );

    virtual ~NavigationState() override;

    virtual void on_activate() override;

    virtual void handle_key_press() override;

    virtual void handle_key_release() override;

    virtual void handle_button_release() override;

    virtual void handle_mouse_move() override;

    virtual void update(double delta_time) override;

private:
    const vsg::ref_ptr<Camera>& camera;
};

#endif // NAVIGATION_STATE_H
