#include "KeyboardHandler.h"

#include "Action.h"
#include "EditorContext.h"
#include "KeyBinding.h"
#include "Settings.h"

#include <vsg/ui/KeyEvent.h>

#include <cstdint>

KeyboardHandler::KeyboardHandler(EditorContext& context)
    : context_(context)
{
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_state_bits_.set(keyPress.keyBase, true);
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_state_bits_.set(keyRelease.keyBase, false);
}

bool KeyboardHandler::get_key_state(vsg::KeySymbol key) const
{
    return key_state_bits_.test(key);
}

bool KeyboardHandler::get_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool KeyboardHandler::get_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) ||
        get_key_state(vsg::KEY_Control_R);
}

bool KeyboardHandler::get_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) || get_key_state(vsg::KEY_Alt_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    const KeyBinding key_binding = context_.settings.key_bindings.at(action);

    uint16_t modifiers = 0;
    modifiers |= (vsg::MODKEY_Shift * get_shift_state());
    modifiers |= (vsg::MODKEY_Control * get_ctrl_state());
    modifiers |= (vsg::MODKEY_Alt * get_alt_state());

    return get_key_state(key_binding.key) && modifiers == key_binding.modifiers;
}
