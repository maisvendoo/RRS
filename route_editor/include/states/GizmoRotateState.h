#ifndef GIZMO_ROTATE_STATE_H
#define GIZMO_ROTATE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

namespace vsg
{

class KeyPressEvent;

}

class GizmoRotateState : public State
{
public:
    GizmoRotateState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~GizmoRotateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // GIZMO_ROTATE_STATE_H
