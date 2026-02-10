#include "KeyboardHandler.h"

#include "Action.h"
#include "KeyBindings.h"
#include "Settings.h"

#include <map>
#include <vsg/ui/KeyEvent.h>

#include <cassert>

KeyboardHandler::KeyboardHandler(const settings_t& settings)
{
    key_bindings[ACTION_MOVE_CAMERA_FORWARD] =
        {KEY_MODIFIER_NONE, settings.key_move_camera_forward};

    key_bindings[ACTION_MOVE_CAMERA_BACKWARD] =
        {KEY_MODIFIER_NONE, settings.key_move_camera_backward};

    key_bindings[ACTION_MOVE_CAMERA_LEFT] =
        {KEY_MODIFIER_NONE, settings.key_move_camera_left};

    key_bindings[ACTION_MOVE_CAMERA_RIGHT] =
        {KEY_MODIFIER_NONE, settings.key_move_camera_right};

    key_bindings[ACTION_MOVE_OBJECTS] =
        {KEY_MODIFIER_NONE, settings.key_move_objects};

    key_bindings[ACTION_ROTATE_OBJECTS] =
        {KEY_MODIFIER_NONE, settings.key_rotate_objects};
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_states[keyPress.keyBase] = true;
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_states[keyRelease.keyBase] = false;
}

KeyBinding KeyboardHandler::get_key_binding(Action action) const
{
    assert(action >= 0);
    assert(action < TOTAL_ACTIONS);

    return key_bindings[action];
}

bool KeyboardHandler::get_key_state(vsg::KeySymbol key) const
{
    const auto found_it = key_states.find(key);
    return (found_it == key_states.cend()) ? false : found_it->second;
}

bool KeyboardHandler::get_any_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool KeyboardHandler::get_any_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) ||
        get_key_state(vsg::KEY_Control_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    assert(action >= 0);
    assert(action < TOTAL_ACTIONS);

    static const std::map<MyKeyModifier, vsg::KeySymbol> modifiers_map = {
        {KEY_MODIFIER_LSHIFT, vsg::KEY_Shift_L},
        {KEY_MODIFIER_RSHIFT, vsg::KEY_Shift_R},
        {KEY_MODIFIER_LCTRL, vsg::KEY_Control_L},
        {KEY_MODIFIER_RCTRL, vsg::KEY_Control_R},
        {KEY_MODIFIER_LALT, vsg::KEY_Alt_L},
        {KEY_MODIFIER_RALT, vsg::KEY_Alt_R}
    };

    const KeyBinding key_binding = get_key_binding(action);

    for (const auto& [modifier, key] : modifiers_map)
    {
        if ((key_binding.modifiers & modifier) && !get_key_state(key))
        {
            return false;
        }
    }

    return get_key_state(key_binding.key);
}

const KeyBindings& KeyboardHandler::get_key_bindings() const
{
    return key_bindings;
}
