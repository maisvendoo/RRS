#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

class Camera;
class CommandList;
class Keyboard;
class Mouse;
class State;

class StateManager
{
public:
    StateManager(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        const vsg::ref_ptr<Camera>& camera,
        CommandList& command_list
    );

    ~StateManager();

    void defer_switch_to_route_not_loaded_state();
    void defer_switch_to_basic_editor_state();
    void defer_switch_to_navigation_state();
    void defer_switch_to_box_selection_state();
    void defer_switch_to_keyboard_translate_state();
    void defer_switch_to_keyboard_rotate_state();
    void defer_switch_to_keyboard_scale_state();
    void defer_switch_to_gizmo_translate_state();
    void defer_switch_to_gizmo_rotate_state();
    void defer_switch_to_gizmo_scale_state();

    void update(double delta_time);

    State* get_editor_state() const;

private:
    State* current_state;
    State* deferred_state;

    State* route_not_loaded_state;
    State* basic_editor_state;
    State* navigation_state;

    State* keyboard_translate_state;
    State* keyboard_rotate_state;
    State* keyboard_scale_state;

    State* gizmo_translate_state;
    State* gizmo_rotate_state;
    State* gizmo_scale_state;
};

#endif // STATE_MANAGER_H
