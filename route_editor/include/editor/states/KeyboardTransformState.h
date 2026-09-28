#ifndef EDITOR_STATES_KEYBOARD_TRANSFORM_STATE_H
#define EDITOR_STATES_KEYBOARD_TRANSFORM_STATE_H

#include "editor/states/State.h"

#include <vsg/maths/vec3.h>

struct EditorContext;

class KeyboardTransformState : public State
{
public:
    KeyboardTransformState(EditorContext& editor_context);
    virtual ~KeyboardTransformState() override;

    virtual void on_activate() override;

    virtual void handle_key_press() override;
    virtual void handle_button_press() override;
    virtual void handle_mouse_move() override;

protected:
    vsg::dvec3 begin_intersection;

protected:
    virtual void confirm_transform() const;
    void cancel_transform() const;

    vsg::dvec3 calculate_world_intersection() const;
};

#endif // EDITOR_STATES_KEYBOARD_TRANSFORM_STATE_H
