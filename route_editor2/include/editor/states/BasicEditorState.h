#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

#include <string>

class Camera;
class Keyboard;

class BasicEditorState : public EditorState
{
public:
    BasicEditorState(
        const vsg::ref_ptr<Keyboard>& keyboard,
        const std::string& route_dir,
        const vsg::ref_ptr<Camera>& camera
    );

    virtual ~BasicEditorState() override;

    virtual void fill_status_bar() const override;

    virtual void handle_key_press() const override;
    virtual void handle_key_release() const override;
    virtual void handle_button_press() const override;
    virtual void handle_button_release() const override;
    virtual void handle_mouse_move() const override;

    virtual void update(double delta_time) const override;

private:
    const vsg::ref_ptr<Keyboard>& keyboard;
    const std::string& route_dir;
    const vsg::ref_ptr<Camera>& camera;
};

#endif // BASIC_EDITOR_STATE_H
