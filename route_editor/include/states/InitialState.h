#ifndef INITIAL_STATE_H
#define INITIAL_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

class InitialState : public State
{
public:
    InitialState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~InitialState() override;
    virtual void handle_key_press() override;
};

#endif // INITIAL_STATE_H
