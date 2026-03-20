#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "Action.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/KeyEvent.h>

#include <cstdint>

struct EditorContext;

class KeyboardHandler : public vsg::Inherit<vsg::Visitor, KeyboardHandler>
{
public:
    explicit KeyboardHandler(EditorContext& context);

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    bool get_key_state(vsg::KeySymbol key) const;
    bool get_any_shift_state() const;
    bool get_any_ctrl_state() const;
    bool get_any_alt_state() const;
    bool get_binding_state(Action action) const;

private:
    EditorContext& context;

    // Bit array for 65536 bits.
    // vsg::KeySymbol is uint16_t with maximum value of 65535,
    // so every bit of this array is state of every possible vsg::KeySymbol.
    // Bit is set like this:
    // Byte index = keyBase >> 3
    // Byte value |= 1 << (keyBase & 0b111)
    // For example:
    // keyBase = 31 = 0b00011111
    // Byte index   = 0b00000011 = 3
    // key_state_bits[3] |= 1 << 7 = 0b10000000
    // keyBase = 32 = 0b00100000
    // Byte index   = 0b00000100 = 4
    // key_state_bits[4] |= 1 << 0 = 0b00000001
    // keyBase = 33 = 0b00100001
    // Byte index   = 0b00000100 = 4
    // key_state_bits[4] |= 1 << 1 = 0b00000010
    std::uint8_t key_state_bits[8192];
};

#endif // KEYBOARD_HANDLER_H
