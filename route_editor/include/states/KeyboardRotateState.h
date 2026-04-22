#ifndef KEYBOARD_ROTATE_STATE_H
#define KEYBOARD_ROTATE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class KeyboardRotateState : public State
{
public:
    virtual ~KeyboardRotateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // KEYBOARD_ROTATE_STATE_H
