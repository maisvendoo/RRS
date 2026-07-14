#include "editor/Action.h"

static constexpr const char* action_names[TOTAL_ACTIONS] = {
    "Move camera forward",
    "Move camera backward",
    "Move camera left",
    "Move camera right",
    "Translate objects",
    "Rotate objects",
    "Scale objects",
    "Copy objects",
    "Paste objects",
    "Hide objects",
    "Show objects",
    "Delete objects",
    "Undo command",
    "Redo command",
    "Save route"
};

const char* to_c_string(Action action)
{
    return action_names[action];
}
