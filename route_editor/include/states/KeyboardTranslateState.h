#ifndef KEYBOARD_TRANSLATE_STATE_H
#define KEYBOARD_TRANSLATE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class KeyboardTranslateState : public State
{
public:
    KeyboardTranslateState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~KeyboardTranslateState() override;

    virtual void handle_key_press() override;
};

#endif // KEYBOARD_TRANSLATE_STATE_H
