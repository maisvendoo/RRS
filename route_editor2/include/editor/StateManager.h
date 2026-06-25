#ifndef EDITOR_STATE_MANAGER_H
#define EDITOR_STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <memory>
#include <string>

class Camera;
class EditorState;
class Keyboard;

class StateManager
{
public:
    StateManager(
        const vsg::ref_ptr<Keyboard>& keyboard,
        const std::string& route_dir,
        const vsg::ref_ptr<Camera>& camera
    );

    ~StateManager();

    const std::unique_ptr<EditorState>& get_editor_state() const;

    void defer_switch_to_route_not_loaded_state();

    void defer_switch_to_basic_editor_state();

    void update();

private:
    std::unique_ptr<EditorState>* editor_state;
    std::unique_ptr<EditorState>* deferred_editor_state;
    std::unique_ptr<EditorState> route_not_loaded_state;
    std::unique_ptr<EditorState> basic_editor_state;
};

#endif // EDITOR_STATE_MANAGER_H
