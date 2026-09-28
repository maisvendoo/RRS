#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <array>
#include <memory>

class BasicEditorState;
struct EditorContext;
class GizmoRotateState;
class GizmoScaleState;
class GizmoTranslateState;
class KeyboardRotateState;
class KeyboardScaleState;
class KeyboardTranslateState;
class NavigationState;
class RouteNotLoadedState;
class State;

enum StateEnum
{
    STATE_ROUTE_NOT_LOADED,
    STATE_BASIC,
    STATE_NAVIGATION,
    STATE_KEYBOARD_TRANSLATE,
    STATE_KEYBOARD_ROTATE,
    STATE_KEYBOARD_SCALE,
    STATE_GIZMO_TRANSLATE,
    STATE_GIZMO_ROTATE,
    STATE_GIZMO_SCALE,
    TOTAL_STATE_COUNT
};

class StateManager
{
public:
    StateManager(EditorContext& editor_context);
    ~StateManager();

    void defer_switch_to(StateEnum state);

    void update(double delta_time);

    const std::unique_ptr<State>& get_current_editor_state() const;

    RouteNotLoadedState* get_route_not_loaded_state() const;
    BasicEditorState* get_basic_editor_state() const;
    NavigationState* get_navigation_state() const;
    KeyboardTranslateState* get_keyboard_translate_state() const;
    KeyboardRotateState* get_keyboard_rotate_state() const;
    KeyboardScaleState* get_keyboard_scale_state() const;
    GizmoTranslateState* get_gizmo_translate_state() const;
    GizmoRotateState* get_gizmo_rotate_state() const;
    GizmoScaleState* get_gizmo_scale_state() const;

private:
    std::array<std::unique_ptr<State>, TOTAL_STATE_COUNT> states;
    std::unique_ptr<State>* current_state;
    std::unique_ptr<State>* deferred_state;
};

#endif // STATE_MANAGER_H
