#ifndef SELECT_ROUTE_STATE_H
#define SELECT_ROUTE_STATE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class SelectRouteState : public State
{
public:
    virtual ~SelectRouteState() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // SELECT_ROUTE_STATE_H
