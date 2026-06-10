#ifndef EDITOR_STATE_MANAGER_H
#define EDITOR_STATE_MANAGER_H

#include <memory>

class EditorState;

class StateManager
{
public:
    StateManager();

    ~StateManager();

    const std::unique_ptr<EditorState>& get_editor_state() const;

    void set_state_route_not_selected();

    void set_state_basic_editor_state();

private:
    std::unique_ptr<EditorState>* editor_state;
    std::unique_ptr<EditorState> route_not_selected_state;
    std::unique_ptr<EditorState> basic_editor_state;
};

#endif // EDITOR_STATE_MANAGER_H
