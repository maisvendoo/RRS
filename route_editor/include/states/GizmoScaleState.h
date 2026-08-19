#ifndef GIZMO_SCALE_STATE_H
#define GIZMO_SCALE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class GizmoScaleState : public State
{
public:
    virtual ~GizmoScaleState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // GIZMO_SCALE_STATE_H
