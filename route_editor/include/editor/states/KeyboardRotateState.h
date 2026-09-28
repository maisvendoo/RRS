#ifndef EDITOR_STATES_KEYBOARD_ROTATE_STATE_H
#define EDITOR_STATES_KEYBOARD_ROTATE_STATE_H

#include "editor/states/KeyboardTransformState.h"

#include <vsg/maths/vec3.h>

class KeyboardRotateState : public KeyboardTransformState
{
public:
    KeyboardRotateState(EditorContext& editor_context);
    virtual ~KeyboardRotateState() override;

    virtual void on_activate() override;

    virtual void handle_mouse_move() override;

private:
    double rotation_rad;
    vsg::dvec3 gizmo_pos;
    vsg::dvec3 camera_front;

private:
    virtual void confirm_transform() const override;
};

#endif // EDITOR_STATES_KEYBOARD_ROTATE_STATE_H
