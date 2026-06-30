#ifndef EDITOR_KEYBOARD_H
#define EDITOR_KEYBOARD_H

#include "editor/Action.h"

#include <vsg/core/Inherit.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/Keyboard.h>

#include <cstdint>

struct KeyBindings;

class Keyboard : public vsg::Inherit<vsg::Keyboard, Keyboard>
{
public:
    explicit Keyboard(const KeyBindings& key_bindings);

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    /**
     * @brief Check if key is pressed once (it is not held down for some time).
     *
     * @param[in] key The keyboard key.
     * @param[in] ignore_handled_keys Ignore keys that already were handled by
     *            other handlers.
     * @return True if key is pressed only once. False - otherwise.
     */
    bool pressed_once(vsg::KeySymbol key, bool ignore_handled_keys = true) const;

    bool pressed(Action action, bool ignore_handled_keys = true) const;

    bool pressed_once(Action action, bool ignore_handled_keys = true) const;

private:
    const KeyBindings& key_bindings;

    std::uint16_t modifiers = 0;

    using vsg::Keyboard::KeyHistory;
    using vsg::Keyboard::keyState;
    using vsg::Keyboard::times;
};

#endif // EDITOR_KEYBOARD_H
