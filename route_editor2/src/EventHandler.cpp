#include "editor/EventHandler.h"

#include "editor/Keyboard.h"
#include "editor/states/EditorState.h"
#include "editor/states/RouteNotSelectedState.h"

#include <memory>

EventHandler::EventHandler()
{
    keyboard = Keyboard::create();
    route_not_selected_state = std::make_unique<RouteNotSelectedState>();
    editor_state = &route_not_selected_state;
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    keyboard->apply(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    keyboard->apply(keyRelease);
}

void EventHandler::apply(vsg::FocusInEvent& focusIn)
{
    keyboard->apply(focusIn);
}

void EventHandler::apply(vsg::FocusOutEvent& focusOut)
{
    keyboard->apply(focusOut);
}
