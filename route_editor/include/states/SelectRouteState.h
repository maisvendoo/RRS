#ifndef SELECT_ROUTE_STATE_H
#define SELECT_ROUTE_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

class SelectRouteState : public State
{
public:
    SelectRouteState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~SelectRouteState() override;
};

#endif // SELECT_ROUTE_STATE_H
