#ifndef EDITOR_KEY_BINDINGS_H
#define EDITOR_KEY_BINDINGS_H

#include "Action.h"

#include <vsg/ui/KeyEvent.h>

#include <cstdint>

class CfgReader;

struct KeyBindings
{
    vsg::KeySymbol keys[TOTAL_ACTIONS];
    std::uint16_t modifiers[TOTAL_ACTIONS];

    KeyBindings();

    void read(CfgReader& cfg);
};

#endif // EDITOR_KEY_BINDINGS_H
