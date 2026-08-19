#ifndef GIZMO_TRANSLATE_STATE_H
#define GIZMO_TRANSLATE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class GizmoTranslateState : public State
{
public:
    virtual ~GizmoTranslateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // GIZMO_TRANSLATE_STATE_H
