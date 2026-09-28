#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "editor/states/State.h"

#include <vsg/core/ref_ptr.h>

struct EditorContext;
class RouteObject;

class BasicEditorState : public State
{
public:
    BasicEditorState(EditorContext& editor_context);
    virtual ~BasicEditorState() override;

    virtual void handle_key_press() override;

    virtual void handle_button_press() override;
    virtual void handle_mouse_scroll() override;

private:
    void save_route();
    void select_object(const vsg::ref_ptr<RouteObject>& object);
};

#endif // BASIC_EDITOR_STATE_H
