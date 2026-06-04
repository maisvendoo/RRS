#include "editor/EventHandler.h"

#include "editor/Keyboard.h"
#include "editor/states/SelectRouteState.h"

#include <memory>

EventHandler::EventHandler()
{
    keyboard = Keyboard::create();
    select_route_state = std::make_unique<SelectRouteState>();
    editor_state = &select_route_state;
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
