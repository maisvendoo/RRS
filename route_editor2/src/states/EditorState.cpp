#include "editor/states/EditorState.h"

#include <vsg/ui/KeyEvent.h>

EditorState::~EditorState() = default;

void EditorState::fill_status_bar() const
{
}

void EditorState::handle_key_press(vsg::KeySymbol key) const
{
}

void EditorState::handle_key_release(vsg::KeySymbol key) const
{
}

void EditorState::handle_mouse_move(int x, int y) const
{
}

void EditorState::update(double delta_time) const
{
}
