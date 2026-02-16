#ifndef KEY_BINDING_H
#define KEY_BINDING_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>
#include <cstdint>

enum EditorKeyModifier
{
    EDITOR_KEY_MODIFIER_SHIFT_L   = 0x0001,
    EDITOR_KEY_MODIFIER_SHIFT_R   = 0x0002,
    EDITOR_KEY_MODIFIER_SHIFT_ANY = 0x0004,
    EDITOR_KEY_MODIFIER_CTRL_L    = 0x0008,
    EDITOR_KEY_MODIFIER_CTRL_R    = 0x0010,
    EDITOR_KEY_MODIFIER_CTRL_ANY  = 0x0020,
    EDITOR_KEY_MODIFIER_ALT_L     = 0x0040,
    EDITOR_KEY_MODIFIER_ALT_R     = 0x0080,
    EDITOR_KEY_MODIFIER_ALT_ANY   = 0x0100
};

struct KeyBinding
{
    vsg::KeySymbol key;
    std::uint32_t modifiers = 0;
};

using KeyBindings = std::array<KeyBinding, TOTAL_ACTIONS>;

#endif // KEY_BINDING_H
