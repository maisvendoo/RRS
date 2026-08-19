#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "Action.h"
#include "KeyBinding.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/KeyEvent.h>

#include <bitset>
#include <cstdint>

class KeyboardHandler : public vsg::Inherit<vsg::Visitor, KeyboardHandler>
{
public:
    explicit KeyboardHandler(const KeyBindings& key_bindings);
    virtual ~KeyboardHandler() = default;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    bool get_key_state(vsg::KeySymbol key) const;
    bool get_shift_state() const;
    bool get_ctrl_state() const;
    bool get_alt_state() const;
    bool get_binding_state(Action action) const;

private:
    const KeyBindings& key_bindings_;
    std::bitset<UINT16_MAX + 1> key_state_bits_;
};

#endif // KEYBOARD_HANDLER_H
