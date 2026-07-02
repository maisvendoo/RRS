#include "editor/states/EditorState.h"

#include <vsg/ui/KeyEvent.h>

EditorState::~EditorState() = default;

void EditorState::fill_status_bar() const
{
}

void EditorState::handle_key_press(vsg::KeySymbol key) const
{
    (void)key;
}

void EditorState::handle_key_release(vsg::KeySymbol key) const
{
    (void)key;
}

void EditorState::handle_mouse_move(int delta_x, int delta_y) const
{
    (void)delta_x;
    (void)delta_y;
}

void EditorState::update(double delta_time) const
{
    (void)delta_time;
}
