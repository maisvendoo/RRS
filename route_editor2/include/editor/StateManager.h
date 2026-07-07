#ifndef EDITOR_STATE_MANAGER_H
#define EDITOR_STATE_MANAGER_H

#include <vsg/core/ref_ptr.h>

#include <array>
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

enum EnumEditorState
{
    EDITOR_STATE_ROUTE_NOT_LOADED,
    EDITOR_STATE_BASIC,
    EDITOR_STATE_NAVIGATION,
    EDITOR_STATE_BOX_SELECTION,
    TOTAL_EDITOR_STATES
};

class StateManager
{
public:
    StateManager(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        const vsg::ref_ptr<Camera>& camera,
        const std::string& route_dir,
        const vsg::ref_ptr<vsg::Options>& vsg_options
    );

    ~StateManager();

    void defer_switch(EnumEditorState state);

    void update(double delta_time);

    const std::unique_ptr<EditorState>& get_editor_state() const;

private:
    const std::unique_ptr<EditorState>* current_state;
    const std::unique_ptr<EditorState>* deferred_state;
    std::array<std::unique_ptr<EditorState>, TOTAL_EDITOR_STATES> editor_states;
};

#endif // EDITOR_STATE_MANAGER_H
