#ifndef EDITOR_KEY_BINDINGS_H
#define EDITOR_KEY_BINDINGS_H

#include "editor/Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>
#include <cstdint>

class CfgReader;

struct KeyBinding
{
    vsg::KeySymbol key;
    std::uint16_t modifiers = 0;
};

struct KeyBindings
{
    std::array<KeyBinding, TOTAL_ACTIONS> bindings;

    void read(CfgReader& cfg);
};

#endif // EDITOR_KEY_BINDINGS_H
