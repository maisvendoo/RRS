#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <array>
#include <memory>

class Camera;
class CommandManager;
class Keyboard;
class Mouse;
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
    StateManager(
        const vsg::ref_ptr<Keyboard>& keyboard,
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Camera>& camera,
        CommandManager& command_manager
    );

    ~StateManager();

    void defer_switch_to(StateEnum state);

    void update(double delta_time);

    const std::unique_ptr<State>& get_editor_state() const;

private:
    std::array<std::unique_ptr<State>, TOTAL_STATE_COUNT> states;
    std::unique_ptr<State>* current_state;
    std::unique_ptr<State>* deferred_state;
};

#endif // STATE_MANAGER_H
