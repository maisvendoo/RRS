#ifndef EDITOR_KEYBOARD_H
#define EDITOR_KEYBOARD_H

#include <vsg/core/Inherit.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/Keyboard.h>

class Keyboard : public vsg::Inherit<vsg::Keyboard, Keyboard>
{
public:
    /**
     * @brief Check if key is pressed once (it is not held down for some time).
     *
     * @param[in] key The keyboard key.
     * @param[in] ignore_handled_keys Ignore keys the already were handled by
     *            other handlers.
     * @return true if key is pressed only once.
     * @return false - otherwise.
     */
    bool pressed_once(vsg::KeySymbol key, bool ignore_handled_keys = true) const;

private:
    using vsg::Keyboard::KeyHistory;
    using vsg::Keyboard::keyState;
    using vsg::Keyboard::times;
};

#endif // EDITOR_KEYBOARD_H
