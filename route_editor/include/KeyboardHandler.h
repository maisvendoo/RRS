#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "Action.h"
#include "KeyBinding.h"
#include "KeyStates.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/KeyEvent.h>

struct settings_t;

class KeyboardHandler : public vsg::Inherit<vsg::Visitor, KeyboardHandler>
{
public:
    explicit KeyboardHandler(const settings_t& settings);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;

    KeyBinding get_key_binding(Action action) const;
    bool get_key_state(vsg::KeySymbol key) const;
    bool get_any_shift_state() const;
    bool get_any_ctrl_state() const;
    bool get_any_alt_state() const;
    bool get_binding_state(Action action) const;

    const KeyBindings& get_key_bindings() const;

private:
    KeyBindings key_bindings;
    KeyStates key_states;
};

#endif // KEYBOARD_HANDLER_H
