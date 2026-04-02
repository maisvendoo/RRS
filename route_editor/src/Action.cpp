#include "Action.h"

#include <cassert>

static const char* const * get_action_names()
{
    static const char* action_names[TOTAL_ACTIONS];

    action_names[ACTION_MOVE_CAMERA_FORWARD] = "Camera: move forward";
    action_names[ACTION_MOVE_CAMERA_BACKWARD] = "Camera: move backward";
    action_names[ACTION_MOVE_CAMERA_LEFT] = "Camera: move left";
    action_names[ACTION_MOVE_CAMERA_RIGHT] = "Camera: move right";
    action_names[ACTION_MOVE_OBJECTS] = "Objects: Grab";
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

    return action_names;
}

const char* to_c_string(Action action)
{
    assert(action < TOTAL_ACTIONS);

    static const char* const * const action_names = get_action_names();

    return action_names[action];
}
