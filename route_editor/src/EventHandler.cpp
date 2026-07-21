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
    state_manager.get_editor_state()->handle_key_press(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    state_manager.get_editor_state()->handle_key_release(keyRelease);
}

void EventHandler::apply(vsg::FrameEvent& frameEvent)
{
    (void)frameEvent;
}
