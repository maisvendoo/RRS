#ifndef GIZMO_ROTATE_STATE_H
#define GIZMO_ROTATE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class GizmoRotateState : public State
{
public:
    virtual ~GizmoRotateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // GIZMO_ROTATE_STATE_H
