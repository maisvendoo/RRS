#ifndef EDITOR_STATES_BASIC_EDITOR_STATE_H
#define EDITOR_STATES_BASIC_EDITOR_STATE_H

#include "editor/states/State.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class Keyboard;
class RouteObject;

namespace vsg
{

class MatrixTransform;

}

class BasicEditorState : public State
{
public:
    BasicEditorState(EditorContext& editor_ctx);
    virtual ~BasicEditorState() override;

    virtual void handle_key_press() override;

    virtual void handle_button_press() override;
    virtual void handle_mouse_scroll() override;

private:
    Keyboard& keyboard;

private:
    void save_route();
    void select_object(const vsg::MatrixTransform* mat_transform);
};

#endif // EDITOR_STATES_BASIC_EDITOR_STATE_H
