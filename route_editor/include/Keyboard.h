#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Action.h"
#include "KeyBinding.h"

#include <vsg/core/Inherit.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/Keyboard.h>

#include <cstdint>

class Keyboard : public vsg::Inherit<vsg::Keyboard, Keyboard>
{
public:
    explicit Keyboard(const KeyBindings& key_bindings);
    virtual ~Keyboard() = default;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    bool get_shift_state() const;
    bool get_ctrl_state() const;
    bool get_alt_state() const;
    bool get_binding_state(Action action) const;

private:
    const KeyBindings& key_bindings_;
    std::uint16_t modifiers = 0;
};

#endif // KEYBOARD_H
