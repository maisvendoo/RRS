#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

class State;

class StateManager
{
public:
    StateManager();
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

    State* select_route_state;
    State* initial_state;
    State* navigation_state;

    State* keyboard_translate_state;
    State* keyboard_rotate_state;
    State* keyboard_scale_state;

    State* gizmo_translate_state;
    State* gizmo_rotate_state;
    State* gizmo_scale_state;
};

#endif // STATE_MANAGER_H
