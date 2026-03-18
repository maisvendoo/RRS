#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "Action.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/KeyEvent.h>

#include <cstdint>
#include <map>

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
    std::map<vsg::KeySymbol, bool> key_states;
    std::uint8_t key_state_bits[8192];
};

#endif // KEYBOARD_HANDLER_H
