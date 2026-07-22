#include "states/BasicEditorState.h"

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::handle_key_press()
{
}

const char* BasicEditorState::get_name() const
{
    return "BasicEditorState";
}
