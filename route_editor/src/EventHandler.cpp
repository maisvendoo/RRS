#include "editor/EventHandler.h"

#include "editor/EditorContext.h"
#include "editor/StateManager.h"
#include "editor/states/State.h"

EventHandler::EventHandler(EditorContext& editor_context)
    : editor_context(editor_context)
{
}

EventHandler::~EventHandler() = default;

void EventHandler::apply([[maybe_unused]] vsg::KeyPressEvent& keyPress)
{
    editor_context.state_manager->get_current_editor_state()->handle_key_press();
}

void EventHandler::apply([[maybe_unused]] vsg::KeyReleaseEvent& keyRelease)
{
    editor_context.state_manager->get_current_editor_state()->handle_key_release();
}

void EventHandler::apply([[maybe_unused]] vsg::ButtonPressEvent& buttonPress)
{
    editor_context.state_manager->get_current_editor_state()->handle_button_press();
}

void EventHandler::apply([[maybe_unused]] vsg::ButtonReleaseEvent& buttonRelease)
{
    editor_context.state_manager->get_current_editor_state()->handle_button_release();
}

void EventHandler::apply([[maybe_unused]] vsg::MoveEvent& moveEvent)
{
    editor_context.state_manager->get_current_editor_state()->handle_mouse_move();
}

void EventHandler::apply([[maybe_unused]] vsg::ScrollWheelEvent& scrollWheel)
{
    editor_context.state_manager->get_current_editor_state()->handle_mouse_scroll();
}

void EventHandler::update(double delta_time)
{
    editor_context.state_manager->get_current_editor_state()->update(delta_time);
}
