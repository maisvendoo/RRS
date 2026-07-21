#ifndef NAVIGATION_STATE_H
#define NAVIGATION_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class NavigationState : public State
{
public:
    virtual ~NavigationState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // NAVIGATION_STATE_H
