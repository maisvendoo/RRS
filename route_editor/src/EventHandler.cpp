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

void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    (void)moveEvent;
}

void EventHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    (void)scrollWheel;
}

void EventHandler::update(double delta_time)
{
    (void)delta_time;
}
