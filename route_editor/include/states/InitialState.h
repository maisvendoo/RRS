#ifndef INITIAL_STATE_H
#define INITIAL_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class InitialState : public State
{
public:
    virtual ~InitialState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // INITIAL_STATE_H
