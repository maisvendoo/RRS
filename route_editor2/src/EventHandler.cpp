#include "editor/EventHandler.h"

#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

EventHandler::EventHandler(StateManager& state_manager)
    : state_manager(state_manager)
{
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::ConfigureWindowEvent& configureWindow)
{
    static_cast<void>(configureWindow);

    state_manager.get_editor_state()->handle_window_resize();
}

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    // if (keyPress.handled)
    // {
    //     return;
    // }
    static_cast<void>(keyPress);

    state_manager.get_editor_state()->handle_key_press();
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    // if (keyRelease.handled)
    // {
    //     return;
    // }
    static_cast<void>(keyRelease);

    state_manager.get_editor_state()->handle_key_release();
}

void EventHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_button_press();
}

void EventHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_button_release();
}

void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_mouse_move();
}
