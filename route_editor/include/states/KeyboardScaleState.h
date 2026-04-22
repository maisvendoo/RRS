#ifndef KEYBOARD_SCALE_STATE_H
#define KEYBOARD_SCALE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class KeyboardScaleState : public State
{
public:
    virtual ~KeyboardScaleState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // KEYBOARD_SCALE_STATE_H
