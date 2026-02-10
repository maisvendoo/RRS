#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>

enum KeyModifier
{
    KEY_MODIFIER_NONE =     0x0000,
    KEY_MODIFIER_LSHIFT =   0x0001,
    KEY_MODIFIER_RSHIFT =   0x0002,
    KEY_MODIFIER_LCTRL =    0x0004,
    KEY_MODIFIER_RCTRL =    0x0008,
    KEY_MODIFIER_LALT =     0x0010,
    KEY_MODIFIER_RALT =     0x0020,

    KEY_MODIFIER_ANYSHIFT = KEY_MODIFIER_LSHIFT | KEY_MODIFIER_RSHIFT,
    KEY_MODIFIER_ANYCTRL =  KEY_MODIFIER_LCTRL | KEY_MODIFIER_RCTRL,
    KEY_MODIFIER_ANYALT =   KEY_MODIFIER_LALT | KEY_MODIFIER_RALT
};

struct KeyBinding
{
    int modifiers = KEY_MODIFIER_NONE;
    vsg::KeySymbol key;
};

using KeyBindings = std::array<KeyBinding, TOTAL_ACTIONS>;

#endif // KEY_BINDINGS_H
