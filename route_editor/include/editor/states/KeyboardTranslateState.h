#ifndef EDITOR_STATES_KEYBOARD_TRANSLATE_STATE_H
#define EDITOR_STATES_KEYBOARD_TRANSLATE_STATE_H

#include "editor/states/TransformState.h"

#include <vsg/maths/vec3.h>

class KeyboardTranslateState : public TransformState
{
public:
    KeyboardTranslateState(EditorContext& editor_context);
    virtual ~KeyboardTranslateState() override;

    virtual void on_activate() override;

    virtual void handle_mouse_move() override;

private:
    vsg::dvec3 prev_intersection;

private:
    virtual void confirm_transform() const override;
};

#endif // EDITOR_STATES_KEYBOARD_TRANSLATE_STATE_H
