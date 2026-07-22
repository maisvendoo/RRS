#ifndef GIZMO_TRANSLATE_STATE_H
#define GIZMO_TRANSLATE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

class GizmoTranslateState : public State
{
public:
    GizmoTranslateState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~GizmoTranslateState() override;
    virtual void handle_key_press() override;
    virtual const char* get_name() const override;
};

#endif // GIZMO_TRANSLATE_STATE_H
