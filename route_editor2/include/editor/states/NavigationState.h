#ifndef EDITOR_NAVIGATION_STATE_H
#define EDITOR_NAVIGATION_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class NavigationState : public EditorState
{
public:
    NavigationState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~NavigationState() override;

    virtual void handle_key_press() const override;

    virtual void handle_key_release() const override;

    virtual void handle_button_release() const override;

    virtual void handle_mouse_move() override;

    virtual void fill_status_bar() const override;
};

#endif // EDITOR_NAVIGATION_STATE_H
