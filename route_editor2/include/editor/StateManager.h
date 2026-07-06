#ifndef EDITOR_STATE_MANAGER_H
#define EDITOR_STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <memory>
#include <string>

class Camera;
class EditorState;
class Keyboard;
class Mouse;

namespace vsg
{

class Options;

}

class StateManager
{
public:
    StateManager(
        const vsg::ref_ptr<const Mouse>& mouse,
        const vsg::ref_ptr<const Keyboard>& keyboard,
        const std::string& route_dir,
        const vsg::ref_ptr<Camera>& camera,
        const vsg::ref_ptr<vsg::Options>& vsg_options
    );

    ~StateManager();

    const std::unique_ptr<EditorState>& get_editor_state() const;

    void defer_switch_to_route_not_loaded_state();

    void defer_switch_to_basic_editor_state();

    void defer_switch_to_box_selection_state();

    void update(double delta_time);

private:
    const vsg::ref_ptr<const Mouse>& mouse;

    std::unique_ptr<EditorState>* editor_state;
    std::unique_ptr<EditorState>* deferred_editor_state;
    std::unique_ptr<EditorState> route_not_loaded_state;
    std::unique_ptr<EditorState> basic_editor_state;
    std::unique_ptr<EditorState> box_selection_state;
};

#endif // EDITOR_STATE_MANAGER_H
