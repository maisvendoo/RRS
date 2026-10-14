#include "editor/Action.h"

#include <array>
#include <string>

static std::array<std::string, TOTAL_ACTIONS> get_action_names()
{
    std::array<std::string, TOTAL_ACTIONS> action_names;
    action_names[ACTION_MOVE_CAMERA_FORWARD] = "Camera: move forward";
    action_names[ACTION_MOVE_CAMERA_BACKWARD] = "Camera: move backward";
    action_names[ACTION_MOVE_CAMERA_LEFT] = "Camera: move left";
    action_names[ACTION_MOVE_CAMERA_RIGHT] = "Camera: move right";
    action_names[ACTION_TRANSLATE_OBJECTS] = "Objects: Translate";
    action_names[ACTION_ROTATE_OBJECTS] = "Objects: Rotate";
    action_names[ACTION_SCALE_OBJECTS] = "Objects: Scale";
    action_names[ACTION_COPY_OBJECTS] = "Objects: Copy";
    action_names[ACTION_PASTE_OBJECTS] = "Objects: Paste";
    action_names[ACTION_HIDE_OBJECTS] = "Objects: Hide";
    action_names[ACTION_SHOW_OBJECTS] = "Objects: Show";
    action_names[ACTION_DELETE_OBJECTS] = "Objects: Delete";
    action_names[ACTION_UNDO_COMMAND] = "Undo command";
    action_names[ACTION_REDO_COMMAND] = "Redo command";
    action_names[ACTION_SAVE_ROUTE] = "Save route";
    action_names[ACTION_SWAP_PROJECTION_MATRIX]
        = "Camera: change projection matrix";
    return action_names;
}

const std::string &to_string(Action action)
{
    static auto action_names = get_action_names();
    return action_names.at(action);
}
