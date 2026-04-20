#ifndef SELECT_ROUTE_H
#define SELECT_ROUTE_H

#include "states/State.h"

namespace vsg
{

class KeyPressEvent;

}

class SelectRoute : public State
{
public:
    virtual ~SelectRoute() override;
    virtual void handle_key_press(vsg::KeyPressEvent& keyPress) override;
};

#endif // SELECT_ROUTE_H
