#include "Action.h"

#include <map>

static const std::map<Action, const char*> action_map = {
    {ACTION_MOVE_CAMERA_FORWARD, "Camera: move forward"},
    {ACTION_MOVE_CAMERA_BACKWARD, "Camera: move backward"},
    {ACTION_MOVE_CAMERA_LEFT, "Camera: move left"},
    {ACTION_MOVE_CAMERA_RIGHT, "Camera: move right"},
    {ACTION_MOVE_OBJECTS, "Objects: Grab"},
    {ACTION_ROTATE_OBJECTS, "Objects: Rotate"},
    {ACTION_SCALE_OBJECTS, "Objects: Scale"},
    {ACTION_COPY_OBJECTS, "Objects: Copy"},
    {ACTION_PASTE_OBJECTS, "Objects: Paste"},
    {ACTION_UNDO_COMMAND, "Undo command"},
    {ACTION_REDO_COMMAND, "Redo command"}
};

const char* to_c_string(Action action)
{
    return action_map.at(action);
}
