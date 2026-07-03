#include "editor/states/EditorState.h"

EditorState::~EditorState() = default;

void EditorState::fill_status_bar() const
{
}

void EditorState::handle_key_press() const
{
}

void EditorState::handle_key_release() const
{
}

void EditorState::handle_button_press() const
{
}

void EditorState::handle_button_release() const
{
}

void EditorState::handle_mouse_move()
{
}

void EditorState::update(double delta_time) const
{
    (void)delta_time;
}
