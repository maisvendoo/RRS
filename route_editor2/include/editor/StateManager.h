#ifndef EDITOR_STATE_MANAGER_H
#define EDITOR_STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <string>

class Camera;
class EditorState;
class Keyboard;
class Mouse;

namespace vsg
{

class Group;
class Options;
class Window;

}

class StateManager
{
public:
    StateManager(
        const vsg::ref_ptr<vsg::Window>& window,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        const vsg::ref_ptr<Camera>& camera,
        const std::string& route_dir,
        const vsg::ref_ptr<vsg::Options>& vsg_options,
        const vsg::ref_ptr<vsg::Group>& gui_group
    );

    ~StateManager();

    void defer_switch_to_route_not_loaded_state();

    void defer_switch_to_basic_editor_state();

    void defer_switch_to_navigation_state();

    void defer_switch_to_box_selection_state();

    void update(double delta_time);

    EditorState* get_editor_state() const;

private:
    EditorState* current_state;
    EditorState* deferred_state;

    EditorState* route_not_loaded_state;
    EditorState* basic_editor_state;
    EditorState* navigation_state;
    EditorState* box_selection_state;
};

#endif // EDITOR_STATE_MANAGER_H
