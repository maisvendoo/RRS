#ifndef GIZMO_ROTATE_STATE_H
#define GIZMO_ROTATE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class GizmoRotateState : public State
{
public:
    GizmoRotateState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~GizmoRotateState() override;

    virtual void handle_key_press() override;
};

#endif // GIZMO_ROTATE_STATE_H
