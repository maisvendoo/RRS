#include "Keyboard.h"

#include "Action.h"
#include "EditorContext.h"
#include "KeyBinding.h"

#include <vsg/ui/KeyEvent.h>

#include <cstdint>

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

bool Keyboard::get_binding_state(Action action) const
{
    const KeyBinding& key_binding = key_bindings_.at(action);
    return (modifiers == key_binding.modifiers) && pressed(key_binding.key);
}
