#include "Keyboard.h"

#include "Action.h"
#include "KeyBinding.h"

#include <vsg/ui/KeyEvent.h>

#include <cstdint>

Keyboard::Keyboard(const KeyBindings* key_bindings)
    : key_bindings_(key_bindings)
{
}

void Keyboard::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    key_states_.set(keyPress.keyBase, true);
}

void Keyboard::handle_key_release(vsg::KeyReleaseEvent& keyRelease)
{
    key_states_.set(keyRelease.keyBase, false);
}

bool Keyboard::get_key_state(vsg::KeySymbol key) const
{
    return key_states_.test(key);
}

bool Keyboard::get_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) || get_key_state(vsg::KEY_Alt_R);
}

bool Keyboard::get_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) || get_key_state(vsg::KEY_Control_R);
}

bool Keyboard::get_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool Keyboard::get_binding_state(Action action) const
{
    const KeyBinding key_binding = key_bindings_->at(action);

    std::uint16_t modifiers = 0;
    modifiers |= (EDITOR_KEY_MODIFIER_ALT * get_alt_state());
    modifiers |= (EDITOR_KEY_MODIFIER_CTRL * get_ctrl_state());
    modifiers |= (EDITOR_KEY_MODIFIER_SHIFT * get_shift_state());

    return get_key_state(key_binding.key) && (modifiers == key_binding.modifiers);
}
