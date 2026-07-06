#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

EditorState::EditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : mouse(mouse)
    , keyboard(keyboard)
    , state_manager(state_manager)
{
}

EditorState::~EditorState() = default;

void EditorState::on_activate()
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
    static_cast<void>(delta_time);
}

void EditorState::fill_status_bar() const
{
}
