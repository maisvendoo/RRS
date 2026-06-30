#include "editor/Action.h"

#include <array>

extern template class std::array<const char*, TOTAL_ACTIONS>;

using ActionNames = std::array<const char*, TOTAL_ACTIONS>;

static ActionNames get_action_names();

const char* to_c_string(Action action)
{
    static ActionNames action_names = get_action_names();
    return action_names[action];
}

ActionNames get_action_names()
{
    ActionNames action_names;

    action_names[ACTION_MOVE_CAMERA_FORWARD] = "Move camera forward";
    action_names[ACTION_MOVE_CAMERA_BACKWARD] = "Move camera backward";
    action_names[ACTION_MOVE_CAMERA_LEFT] = "Move camera left";
    action_names[ACTION_MOVE_CAMERA_RIGHT] = "Move camera right";
    action_names[ACTION_TRANSLATE_OBJECTS] = "Translate objects";
    action_names[ACTION_ROTATE_OBJECTS] = "Rotate objects";
    action_names[ACTION_SCALE_OBJECTS] = "Scale objects";
    action_names[ACTION_COPY_OBJECTS] = "Copy objects";
    action_names[ACTION_PASTE_OBJECTS] = "Paste objects";
    action_names[ACTION_HIDE_OBJECTS] = "Hide objects";
    action_names[ACTION_SHOW_OBJECTS] = "Show objects";
    action_names[ACTION_DELETE_OBJECTS] = "Delete objects";
    action_names[ACTION_UNDO_COMMAND] = "Undo command";
    action_names[ACTION_REDO_COMMAND] = "Redo command";
    action_names[ACTION_SAVE_ROUTE] = "Save route";

    return action_names;
}
