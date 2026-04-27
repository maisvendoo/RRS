#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Action.h"
#include "KeyBinding.h"

#include <bitset>
#include <cstdint>

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class Keyboard
{
public:
    explicit Keyboard(const KeyBindings* key_bindings);

    void handle_key_press(vsg::KeyPressEvent& keyPress);
    void handle_key_release(vsg::KeyReleaseEvent& keyRelease);

    bool get_action_state(Action action) const;
    bool get_alt_state() const;
    bool get_ctrl_state() const;
    bool get_shift_state() const;

private:
    const KeyBindings* const key_bindings_;

    std::bitset<UINT16_MAX + 1> key_states_;
    std::uint16_t active_modifiers_;
};

#endif // KEYBOARD_H
