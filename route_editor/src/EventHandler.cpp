#include "editor/EventHandler.h"

#include "editor/StateManager.h"
#include "editor/states/State.h"

EventHandler::EventHandler(StateManager& state_manager)
    : state_manager(state_manager)
{
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
    state_manager.get_curr_state()->handle_key_press();
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    (void)keyRelease;
    state_manager.get_curr_state()->handle_key_release();
}

void EventHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    (void)buttonPress;
    state_manager.get_curr_state()->handle_button_press();
}

void EventHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    (void)buttonRelease;
    state_manager.get_curr_state()->handle_button_release();
}

void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    (void)moveEvent;
    state_manager.get_curr_state()->handle_mouse_move();
}

void EventHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    (void)scrollWheel;
    state_manager.get_curr_state()->handle_mouse_scroll();
}

void EventHandler::update(double delta_time)
{
    state_manager.get_curr_state()->update(delta_time);
}
