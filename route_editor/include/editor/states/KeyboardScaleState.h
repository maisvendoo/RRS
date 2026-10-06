#ifndef EDITOR_STATES_KEYBOARD_SCALE_STATE_H
#define EDITOR_STATES_KEYBOARD_SCALE_STATE_H

#include "editor/states/TransformState.h"

#include <vsg/maths/vec3.h>

class KeyboardScaleState : public TransformState
{
public:
    KeyboardScaleState(EditorContext& editor_context);
    virtual ~KeyboardScaleState() override;

    virtual void on_activate() override;

    virtual void handle_mouse_move() override;

private:
    vsg::dvec3 scale;
    vsg::dvec3 gizmo_pos;

private:
    virtual void confirm_transform() const override;
};

#endif // EDITOR_STATES_KEYBOARD_SCALE_STATE_H
