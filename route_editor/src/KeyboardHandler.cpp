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
        settings.key_move_camera_forward;

    key_bindings[ACTION_MOVE_CAMERA_BACKWARD] =
        settings.key_move_camera_backward;

    key_bindings[ACTION_MOVE_CAMERA_LEFT] =
        settings.key_move_camera_left;

    key_bindings[ACTION_MOVE_CAMERA_RIGHT] =
        settings.key_move_camera_right;

    key_bindings[ACTION_MOVE_OBJECTS] =
        settings.key_move_objects;

    key_bindings[ACTION_ROTATE_OBJECTS] =
        settings.key_rotate_objects;
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

bool KeyboardHandler::get_any_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) ||
        get_key_state(vsg::KEY_Alt_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    assert(action >= 0);
    assert(action < TOTAL_ACTIONS);

    const KeyBinding key_binding = get_key_binding(action);
    if (!get_key_state(key_binding.key))
    {
        return false;
    }

    static const std::map<int, std::vector<vsg::KeySymbol>> modifiers_map = {
        {MY_KEY_MODIFIER_NONE, {}},
        {MY_KEY_MODIFIER_LSHIFT, {vsg::KEY_Shift_L}},
        {MY_KEY_MODIFIER_RSHIFT, {vsg::KEY_Shift_R}},
        {MY_KEY_MODIFIER_LCTRL, {vsg::KEY_Control_L}},
        {MY_KEY_MODIFIER_RCTRL, {vsg::KEY_Control_R}},
        {MY_KEY_MODIFIER_LALT, {vsg::KEY_Alt_L}},
        {MY_KEY_MODIFIER_RALT, {vsg::KEY_Alt_R}},
        {MY_KEY_MODIFIER_ANYSHIFT, {vsg::KEY_Shift_L, vsg::KEY_Shift_R}},
        {MY_KEY_MODIFIER_ANYCTRL, {vsg::KEY_Control_L, vsg::KEY_Control_R}},
        {MY_KEY_MODIFIER_ANYALT, {vsg::KEY_Alt_L, vsg::KEY_Alt_R}}
    };

    const auto& modifiers = modifiers_map.at(key_binding.modifiers);
    if (modifiers.empty())
    {
        return true;
    }

    for (const vsg::KeySymbol modifier : modifiers)
    {
        if (get_key_state(modifier))
        {
            return true;
        }
    }

    return false;
}

const KeyBindings& KeyboardHandler::get_key_bindings() const
{
    return key_bindings;
}
