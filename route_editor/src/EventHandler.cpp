#include "EventHandler.h"

#include "states/State.h"

EventHandler::EventHandler()
    : state(&select_route_state)
{
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    state->handle_key_press(keyPress);
}
