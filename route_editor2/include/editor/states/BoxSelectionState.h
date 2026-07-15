#ifndef EDITOR_BOX_SELECTION_STATE_H
#define EDITOR_BOX_SELECTION_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/Value.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec2.h>

#include <memory>

class Camera;
class Keyboard;
class Mouse;
class ObjectManager;
class StateManager;

namespace vsg
{

class Group;
class Options;
class StateGroup;
class Switch;
class Window;

};

class BoxSelectionState : public EditorState
{
public:
    int begin_x;
    int begin_y;
    int end_x;
    int end_y;

public:
    BoxSelectionState(
        const vsg::ref_ptr<vsg::Window>& window,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager,
        const vsg::ref_ptr<vsg::Options>& vsg_options,
        const vsg::ref_ptr<vsg::Group>& gui_group,
        const std::unique_ptr<ObjectManager>& object_manager,
        const vsg::ref_ptr<Camera>& camera
    );

    virtual ~BoxSelectionState() override;

    virtual void on_activate() override;

    virtual void on_deactivate() override;

    virtual void handle_window_resize() const override;

    virtual void handle_button_release() const override;

    virtual void handle_mouse_move() override;

    virtual void fill_status_bar() const override;

private:
    struct Transform
    {
        vsg::vec2 translation;
        vsg::vec2 scale;
    };

private:
    const std::unique_ptr<ObjectManager>& object_manager;
    const vsg::ref_ptr<Camera>& camera;

    vsg::ref_ptr<vsg::Switch> switch_node;
    vsg::ref_ptr<vsg::StateGroup> state_group;
    vsg::ref_ptr<vsg::Value<Transform>> transform_value;

private:
    void update_selection();
};

#endif // EDITOR_BOX_SELECTION_STATE_H
