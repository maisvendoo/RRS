#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Action.h"
#include "KeyBinding.h"

#include <vsg/ui/KeyEvent.h>

#include <bitset>
#include <cstdint>

class Keyboard
{
public:
    explicit Keyboard(const KeyBindings* key_bindings);

    void handle_key_press(vsg::KeyPressEvent& keyPress);
    void handle_key_release(vsg::KeyReleaseEvent& keyRelease);

    bool get_key_state(vsg::KeySymbol key) const;
    bool get_alt_state() const;
    bool get_ctrl_state() const;
    bool get_shift_state() const;
    bool get_binding_state(Action action) const;

private:
    const KeyBindings* key_bindings_;

    std::bitset<UINT16_MAX + 1> key_states_;
};

#endif // KEYBOARD_H
