#include "KeyboardHandler.h"

#include "Action.h"
#include "KeyStates.h"
#include "Settings.h"

#include <vsg/ui/KeyEvent.h>

#include <cassert>

KeyboardHandler::KeyboardHandler(const settings_t& settings)
{
    key_bindings[ACTION_MOVE_CAMERA_FORWARD] = settings.key_move_camera_forward;
    key_bindings[ACTION_MOVE_CAMERA_BACKWARD] = settings.key_move_camera_backward;
    key_bindings[ACTION_MOVE_CAMERA_LEFT] = settings.key_move_camera_left;
    key_bindings[ACTION_MOVE_CAMERA_RIGHT] = settings.key_move_camera_right;
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_states[keyPress.keyBase] = true;
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_states[keyRelease.keyBase] = false;
}

vsg::KeySymbol KeyboardHandler::get_key_binding(Action action) const
{
    assert(action < TOTAL_ACTIONS);

    return key_bindings[action];
}

bool KeyboardHandler::get_key_state(vsg::KeySymbol sym) const
{
    const auto& it = key_states.find(sym);
    if (it != key_states.cend())
    {
        return it->second;
    }
    else
    {
        return false;
    }
}

bool KeyboardHandler::get_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    assert(action < TOTAL_ACTIONS);

    return get_key_state(get_key_binding(action));
}

const vsg::KeySymbol* KeyboardHandler::get_key_bindings() const
{
    return key_bindings;
}
