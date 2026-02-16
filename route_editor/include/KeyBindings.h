#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>

enum MyKeyModifier
{
    MY_KEY_MODIFIER_NONE      = 0x0000,
    MY_KEY_MODIFIER_SHIFT_L   = 0x0001,
    MY_KEY_MODIFIER_SHIFT_R   = 0x0002,
    MY_KEY_MODIFIER_SHIFT_ANY = 0x0004,
    MY_KEY_MODIFIER_CTRL_L    = 0x0008,
    MY_KEY_MODIFIER_CTRL_R    = 0x0010,
    MY_KEY_MODIFIER_CTRL_ANY  = 0x0020,
    MY_KEY_MODIFIER_ALT_L     = 0x0040,
    MY_KEY_MODIFIER_ALT_R     = 0x0080,
    MY_KEY_MODIFIER_ALT_ANY   = 0x0100
};

struct KeyBinding
{
    int modifiers = MY_KEY_MODIFIER_NONE;
    vsg::KeySymbol key;
};

using KeyBindings = std::array<KeyBinding, TOTAL_ACTIONS>;

#endif // KEY_BINDINGS_H
