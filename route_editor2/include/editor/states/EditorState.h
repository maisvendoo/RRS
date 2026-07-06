#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class EditorState
{
public:
    EditorState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<const Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~EditorState();

    virtual void on_activate();

    virtual void handle_key_press() const;

    virtual void handle_key_release() const;

    virtual void handle_button_press() const;

    virtual void handle_button_release() const;

    virtual void handle_mouse_move();

    virtual void update(double delta_time) const;

    virtual void fill_status_bar() const;

protected:
    const vsg::ref_ptr<Mouse>& mouse;
    const vsg::ref_ptr<const Keyboard>& keyboard;
    StateManager& state_manager;
};

#endif // EDITOR_STATE_H
