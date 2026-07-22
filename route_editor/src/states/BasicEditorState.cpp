#include "states/BasicEditorState.h"

#include <vsg/ui/PointerEvent.h>

#include "Mouse.h"

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

void BasicEditorState::handle_button_press()
{
    if (mouse->get_button_mask() == vsg::BUTTON_MASK_3)
    {
        // TODO
    }
}

const char* BasicEditorState::get_name() const
{
    return "BasicEditorState";
}
