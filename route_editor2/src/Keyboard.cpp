#include "editor/Keyboard.h"

#include "editor/Action.h"
#include "editor/KeyBindings.h"

#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/Keyboard.h>

Keyboard::Keyboard(const KeyBindings& key_bindings)
    : key_bindings(key_bindings)
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
    const KeyBinding& binding = key_bindings.bindings[action];

    return (binding.modifiers == modifiers) &&
        vsg::Keyboard::pressed(binding.key, ignore_handled_keys);
}

bool Keyboard::pressed_once(Action action, bool ignore_handled_keys) const
{
    const KeyBinding& binding = key_bindings.bindings[action];

    return (binding.modifiers == modifiers) &&
        pressed_once(binding.key, ignore_handled_keys);
}
