#include "EventHandler.h"

#include "StateManager.h"
#include "states/State.h"

EventHandler::EventHandler(StateManager& state_manager)
    : state_manager(state_manager)
{
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    static_cast<void>(keyPress);
    state_manager.get_editor_state()->handle_key_press();
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    static_cast<void>(keyRelease);
    state_manager.get_editor_state()->handle_key_release();
}

void EventHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    static_cast<void>(buttonPress);
    state_manager.get_editor_state()->handle_button_press();
}

void EventHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    static_cast<void>(buttonRelease);
    state_manager.get_editor_state()->handle_button_release();
}


void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    static_cast<void>(moveEvent);
    state_manager.get_editor_state()->handle_mouse_move();
}

void EventHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    static_cast<void>(scrollWheel);
    state_manager.get_editor_state()->handle_mouse_scroll();
}

void EventHandler::update(double delta_time)
{
    state_manager.get_editor_state()->update(delta_time);
}
