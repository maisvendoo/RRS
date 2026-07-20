#include "Action.h"

static constexpr const char* action_names[TOTAL_ACTIONS] = {
    "Camera: move forward",
    "Camera: move backward",
    "Camera: move left",
    "Camera: move right",
    "Objects: Translate",
    "Objects: Rotate",
    "Objects: Scale",
    "Objects: Copy",
    "Objects: Paste",
    "Objects: Hide",
    "Objects: Show",
    "Objects: Delete",
    "Undo command",
    "Redo command",
    "Save route",
    "Camera: change projection matrix"
};

const char* to_c_string(Action action)
{
    return action_names[action];
}
