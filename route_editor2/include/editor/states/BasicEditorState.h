#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/KeyEvent.h>

#include <string>

class Keyboard;

class BasicEditorState : public EditorState
{
public:
    explicit BasicEditorState(const vsg::ref_ptr<Keyboard>& keyboard, const std::string& route_dir);

    virtual ~BasicEditorState() override;

    virtual void fill_status_bar() const override;

    virtual void handle_key_press(vsg::KeySymbol key) const override;

    virtual void handle_key_release(vsg::KeySymbol key) const override;

private:
    const vsg::ref_ptr<Keyboard>& keyboard;
    const std::string& route_dir;
};

#endif // BASIC_EDITOR_STATE_H
