#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

#include <memory>

class SelectRoute;
class State;

namespace vsg
{

class KeyPressEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    EventHandler();
    virtual void apply(vsg::KeyPressEvent& keyPress) override;

private:
    std::unique_ptr<SelectRoute> select_route;
    std::unique_ptr<State>* state;
};

#endif // EVENT_HANDLER_H
