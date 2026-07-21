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
}

void Keyboard::apply(vsg::KeyReleaseEvent& keyRelease)
{
    vsg::Keyboard::apply(keyRelease);
}

bool Keyboard::get_key_state(vsg::KeySymbol key) const
{
    return pressed(key, false);
}

bool Keyboard::get_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool Keyboard::get_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) ||
        get_key_state(vsg::KEY_Control_R);
}

bool Keyboard::get_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) || get_key_state(vsg::KEY_Alt_R);
}

bool Keyboard::get_binding_state(Action action) const
{
    const KeyBinding key_binding = key_bindings_.at(action);

    uint16_t modifiers = 0;
    modifiers |= (vsg::MODKEY_Shift * get_shift_state());
    modifiers |= (vsg::MODKEY_Control * get_ctrl_state());
    modifiers |= (vsg::MODKEY_Alt * get_alt_state());

    return get_key_state(key_binding.key) && modifiers == key_binding.modifiers;
}
