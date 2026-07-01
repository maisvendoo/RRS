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

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    if (keyPress.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_key_press(keyPress.keyBase);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    if (keyRelease.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_key_release(keyRelease.keyBase);
}

void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    state_manager.get_editor_state()->handle_mouse_move(moveEvent.x, moveEvent.y);
}
