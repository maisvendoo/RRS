#ifndef KEYBOARD_ROTATE_STATE_H
#define KEYBOARD_ROTATE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

namespace vsg
{

class KeyPressEvent;

}

class KeyboardRotateState : public State
{
public:
    KeyboardRotateState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~KeyboardRotateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // KEYBOARD_ROTATE_STATE_H
