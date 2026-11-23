#ifndef EDITOR_KEY_BINDINGS_H
#define EDITOR_KEY_BINDINGS_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <array>
#include <cstdint>

class CfgReader;

struct KeyBindings
{
    std::array<vsg::KeySymbol, NUM_ACTIONS> keys;
    std::array<std::uint16_t, NUM_ACTIONS> modifiers;

    KeyBindings();

    void read(CfgReader& cfg);
};

#endif // EDITOR_KEY_BINDINGS_H
