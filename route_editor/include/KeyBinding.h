#ifndef KEY_BINDING_H
#define KEY_BINDING_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>
#include <cstdint>

enum EditorKeyModifier
{
    EDITOR_KEY_MODIFIER_SHIFT = 0x01,
    EDITOR_KEY_MODIFIER_CTRL  = 0x02,
    EDITOR_KEY_MODIFIER_ALT   = 0x04
};

struct KeyBinding
{
    vsg::KeySymbol key;
    std::uint16_t modifiers = 0;
};

using KeyBindings = std::array<KeyBinding, TOTAL_ACTIONS>;

#endif // KEY_BINDING_H
