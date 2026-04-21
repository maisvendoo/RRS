#include "EventHandler.h"

#include "Keyboard.h"
#include "states/State.h"

EventHandler::EventHandler(Keyboard* keyboard)
    : keyboard_(keyboard)
{
    state_ = &select_route_state_;
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    keyboard_->handle_key_press(keyPress);
    state_->handle_key_press(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    keyboard_->handle_key_release(keyRelease);
}
