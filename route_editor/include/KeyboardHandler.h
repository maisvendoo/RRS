#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "Action.h"
#include "EditorContext.h"
#include "KeyStates.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/KeyEvent.h>

class CommandList;

class KeyboardHandler : public vsg::Inherit<vsg::Visitor, KeyboardHandler>
{
public:
    KeyboardHandler(EditorContext& context);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;

    bool get_key_state(vsg::KeySymbol key) const;
    bool get_any_shift_state() const;
    bool get_any_ctrl_state() const;
    bool get_any_alt_state() const;
    bool get_binding_state(Action action) const;

private:
    EditorContext& context;
    KeyStates key_states;
};

#endif // KEYBOARD_HANDLER_H
