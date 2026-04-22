#ifndef KEYBOARD_TRANSLATE_STATE_H
#define KEYBOARD_TRANSLATE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class KeyboardTranslateState : public State
{
public:
    virtual ~KeyboardTranslateState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // KEYBOARD_TRANSLATE_STATE_H
