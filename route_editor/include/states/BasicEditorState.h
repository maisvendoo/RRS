#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Camera;
class CommandManager;
class Keyboard;
class Mouse;
class StateManager;

class BasicEditorState : public State
{
public:
    BasicEditorState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager,
        const vsg::ref_ptr<Camera>& camera,
        CommandManager& command_manager
    );

    virtual ~BasicEditorState() override;

    virtual void handle_key_press() override;

    virtual void handle_button_press() override;

    virtual void handle_mouse_scroll() override;

private:
    const vsg::ref_ptr<Camera>& camera;
    CommandManager& command_manager;
};

#endif // BASIC_EDITOR_STATE_H
