#ifndef GIZMO_SCALE_STATE_H
#define GIZMO_SCALE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

namespace vsg
{

class KeyPressEvent;

}

class GizmoScaleState : public State
{
public:
    GizmoScaleState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~GizmoScaleState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // GIZMO_SCALE_STATE_H
