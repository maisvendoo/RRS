#ifndef EDITOR_STATES_BASIC_EDITOR_STATE_H
#define EDITOR_STATES_BASIC_EDITOR_STATE_H

#include "editor/states/State.h"

#include <vsg/core/ref_ptr.h>

class Camera;
class CommandManager;
struct EditorContext;
class Keyboard;
class Mouse;
class RouteObject;

class BasicEditorState : public State
{
public:
    BasicEditorState(EditorContext& editor_ctx);
    virtual ~BasicEditorState() override;

    virtual void handle_key_press() override;

    virtual void handle_button_press() override;
    virtual void handle_mouse_scroll() override;

private:
    const Keyboard& keyboard;
    const Mouse& mouse;
    Camera& camera;
    CommandManager& command_manager;

private:
    void save_route();
    void select_object(const vsg::ref_ptr<RouteObject>& object);
};

#endif // EDITOR_STATES_BASIC_EDITOR_STATE_H
