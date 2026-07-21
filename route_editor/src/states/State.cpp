#include "states/State.h"

#include <vsg/core/ref_ptr.h>

State::State(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : mouse(mouse)
    , keyboard(keyboard)
{
}

State::~State() = default;

void State::handle_key_press()
{
}

void State::handle_key_release()
{
}
