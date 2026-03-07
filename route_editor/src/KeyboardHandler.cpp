#include "KeyboardHandler.h"

#include "Action.h"
#include "CommandList.h"
#include "EditorContext.h"
#include "KeyBinding.h"
#include "Settings.h"

#include <vsg/ui/KeyEvent.h>

#include <map>

KeyboardHandler::KeyboardHandler(EditorContext& context)
    : context(context)
{
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_states[keyPress.keyBase] = true;

    if (get_binding_state(ACTION_UNDO_COMMAND))
    {
        context.commands.undo();
    }
    else if (get_binding_state(ACTION_REDO_COMMAND))
    {
        context.commands.redo();
    }
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_states[keyRelease.keyBase] = false;
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
    const KeyBinding key_binding = context.settings.key_bindings.at(action);

    if (!get_key_state(key_binding.key))
    {
        return false;
    }

    // Walk through every binding's modifier and check it's keys
    // For example:
    // If binding's modifier = LShift + RShift, action will be performed
    // only when we hold both Shifts;
    // If binding's modifier = Shift(any), action will be performed
    // when we hold any of Shifts
    for (const auto& [modifier, keys] : modifier_keys_map)
    {
        if (key_binding.modifiers & modifier)
        {
            bool pressed = false;
            for (const vsg::KeySymbol key : keys)
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
