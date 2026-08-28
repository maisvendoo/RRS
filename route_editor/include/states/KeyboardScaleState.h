#ifndef KEYBOARD_SCALE_STATE_H
#define KEYBOARD_SCALE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class KeyboardScaleState : public State
{
public:
    KeyboardScaleState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~KeyboardScaleState() override;

    virtual void handle_key_press() override;
};

#endif // KEYBOARD_SCALE_STATE_H
