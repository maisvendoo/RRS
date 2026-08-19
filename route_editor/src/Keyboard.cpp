#include "Keyboard.h"

#include "Action.h"
#include "KeyBinding.h"

#include <vsg/ui/KeyEvent.h>

#define HANDLE_KEY_MODIFIERS(event, modifiers, op)    \
    switch (event.keyBase)                            \
    {                                                 \
        case vsg::KEY_Alt_L:                          \
        case vsg::KEY_Alt_R:                          \
        {                                             \
            modifiers op vsg::MODKEY_Alt;             \
            return;                                   \
        }                                             \
        case vsg::KEY_Control_L:                      \
        case vsg::KEY_Control_R:                      \
        {                                             \
            modifiers op vsg::MODKEY_Control;         \
            return;                                   \
        }                                             \
        case vsg::KEY_Shift_L:                        \
        case vsg::KEY_Shift_R:                        \
        {                                             \
            modifiers op vsg::MODKEY_Shift;           \
            return;                                   \
        }                                             \
        default:                                      \
        {                                             \
            return;                                   \
        }                                             \
    }

Keyboard::Keyboard(const KeyBindings& key_bindings)
    : key_bindings_{key_bindings}
    , active_modifiers_{0}
{
}

void Keyboard::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    key_states_.set(keyPress.keyBase, true);
    HANDLE_KEY_MODIFIERS(keyPress, active_modifiers_, |=);
}

void Keyboard::handle_key_release(vsg::KeyReleaseEvent& keyRelease)
{
    key_states_.set(keyRelease.keyBase, false);
    HANDLE_KEY_MODIFIERS(keyRelease, active_modifiers_, ^=);
}

bool Keyboard::get_action_state(Action action) const
{
    const KeyBinding key_binding{key_bindings_.at(action)};
    return key_states_.test(key_binding.key) && (active_modifiers_ == key_binding.modifiers);
}

bool Keyboard::get_alt_state() const
{
    return active_modifiers_ & vsg::MODKEY_Alt;
}

bool Keyboard::get_ctrl_state() const
{
    return active_modifiers_ & vsg::MODKEY_Control;
}

bool Keyboard::get_shift_state() const
{
    return active_modifiers_ & vsg::MODKEY_Shift;
}
