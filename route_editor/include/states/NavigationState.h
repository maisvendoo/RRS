#ifndef NAVIGATION_STATE_H
#define NAVIGATION_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

namespace vsg
{

class KeyPressEvent;

}

class NavigationState : public State
{
public:
    NavigationState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~NavigationState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // NAVIGATION_STATE_H
