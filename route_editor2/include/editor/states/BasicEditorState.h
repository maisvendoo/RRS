#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

#include <string>

class Keyboard;
class Mouse;
class StateManager;

namespace vsg
{

class Window;

}

class BasicEditorState : public EditorState
{
public:
    BasicEditorState(
        const vsg::ref_ptr<vsg::Window>& window,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager,
        const std::string& route_dir
    );

    virtual ~BasicEditorState() override;

    virtual void handle_key_press() const override;

    virtual void handle_key_release() const override;

    virtual void handle_button_press() const override;

    virtual void handle_button_release() const override;

    virtual void handle_mouse_move() override;

    virtual void update(double delta_time) const override;

    virtual void fill_status_bar() const override;

private:
    const std::string& route_dir;
};

#endif // BASIC_EDITOR_STATE_H
