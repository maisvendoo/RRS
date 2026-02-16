#include "KeyboardHandler.h"

#include "Action.h"
#include "KeyBinding.h"
#include "Settings.h"

#include <map>
#include <vsg/ui/KeyEvent.h>

#include <cassert>

KeyboardHandler::KeyboardHandler(const settings_t& settings)
{
    key_bindings = settings.key_bindings;
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

    static const std::map<EditorKeyModifier, std::vector<vsg::KeySymbol>> modifiers_map = {
        {EDITOR_KEY_MODIFIER_SHIFT_L, {vsg::KEY_Shift_L}},
        {EDITOR_KEY_MODIFIER_SHIFT_R, {vsg::KEY_Shift_R}},
        {EDITOR_KEY_MODIFIER_SHIFT_ANY, {vsg::KEY_Shift_L, vsg::KEY_Shift_R}},
        {EDITOR_KEY_MODIFIER_CTRL_L, {vsg::KEY_Control_L}},
        {EDITOR_KEY_MODIFIER_CTRL_R, {vsg::KEY_Control_R}},
        {EDITOR_KEY_MODIFIER_CTRL_ANY, {vsg::KEY_Control_L, vsg::KEY_Control_R}},
        {EDITOR_KEY_MODIFIER_ALT_L, {vsg::KEY_Alt_L}},
        {EDITOR_KEY_MODIFIER_ALT_R, {vsg::KEY_Alt_R}},
        {EDITOR_KEY_MODIFIER_ALT_ANY, {vsg::KEY_Alt_L, vsg::KEY_Alt_R}}
    };

    for (const auto& [modifier, keys] : modifiers_map)
    {
        if (key_binding.modifiers & modifier)
        {
            bool pressed = false;
            for (const auto& key : keys)
            {
                if (get_key_state(key))
                {
                    pressed = true;
                    break;
                }
            }
            if (!pressed)
            {
                return false;
            }
        }
    }

    return true;
}

const KeyBindings& KeyboardHandler::get_key_bindings() const
{
    return key_bindings;
}
