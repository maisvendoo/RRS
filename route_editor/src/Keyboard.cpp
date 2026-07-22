#include "Keyboard.h"

#include "Action.h"
#include "KeyBindings.h"

#include <vsg/ui/KeyEvent.h>

Keyboard::Keyboard(const KeyBindings& key_bindings)
    : key_bindings_{key_bindings}
{
}

void Keyboard::apply(vsg::KeyPressEvent& keyPress)
{
    vsg::Keyboard::apply(keyPress);
    modifiers = keyPress.keyModifier & (
        vsg::MODKEY_Alt | vsg::MODKEY_Control | vsg::MODKEY_Shift
    );
}

void Keyboard::apply(vsg::KeyReleaseEvent& keyRelease)
{
    vsg::Keyboard::apply(keyRelease);
    modifiers = keyRelease.keyModifier & (
        vsg::MODKEY_Alt | vsg::MODKEY_Control | vsg::MODKEY_Shift
    );
}

bool Keyboard::get_shift_state() const
{
    return modifiers & vsg::MODKEY_Shift;
}

bool Keyboard::get_ctrl_state() const
{
    return modifiers & vsg::MODKEY_Control;
}

bool Keyboard::get_alt_state() const
{
    return modifiers & vsg::MODKEY_Alt;
}

bool Keyboard::pressed_once(vsg::KeySymbol key, bool ignore_handled_keys) const
{
    auto itr = keyState.find(key);
    if (itr == keyState.end())
    {
        return false;
    }

    const auto& keyHistory = itr->second;

    return (keyHistory.timeOfKeyRelease == keyHistory.timeOfFirstKeyPress) &&
        (keyHistory.timeOfLastKeyPress == keyHistory.timeOfFirstKeyPress) &&
        (!(ignore_handled_keys && keyHistory.handled));
}

bool Keyboard::pressed(Action action, bool ignore_handled_keys) const
{
    return (key_bindings_.modifiers[action] == modifiers) &&
        vsg::Keyboard::pressed(key_bindings_.keys[action], ignore_handled_keys);
}

bool Keyboard::pressed_once(Action action, bool ignore_handled_keys) const
{
    return (key_bindings_.modifiers[action] == modifiers) &&
        pressed_once(key_bindings_.keys[action], ignore_handled_keys);
}
