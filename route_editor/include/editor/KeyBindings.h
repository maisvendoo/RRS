#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

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

#endif // KEY_BINDINGS_H
