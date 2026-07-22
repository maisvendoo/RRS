#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;

class BasicEditorState : public State
{
public:
    BasicEditorState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard
    );
    virtual ~BasicEditorState() override;
    virtual void handle_key_press() override;
    virtual void handle_button_press() override;
    virtual const char* get_name() const override;
};

#endif // BASIC_EDITOR_STATE_H
