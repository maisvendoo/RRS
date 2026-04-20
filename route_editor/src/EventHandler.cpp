#include "EventHandler.h"

#include "states/State.h"

EventHandler::EventHandler()
    : state_(nullptr)
{
    state_ = &select_route_state_;
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    state_->handle_key_press(keyPress);
}
