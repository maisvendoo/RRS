#include "Action.h"

#include <iterator>
#include <map>

using ActionNameMap = std::map<Action, const char*>;
using ActionNamePair = ActionNameMap::value_type;

static constexpr ActionNamePair action_name_map_data[] = {
    {ACTION_MOVE_CAMERA_FORWARD,  "Camera: move forward"},
    {ACTION_MOVE_CAMERA_BACKWARD, "Camera: move backward"},
    {ACTION_MOVE_CAMERA_LEFT,     "Camera: move left"},
    {ACTION_MOVE_CAMERA_RIGHT,    "Camera: move right"},

    {ACTION_MOVE_OBJECTS,   "Objects: Grab"},
    {ACTION_ROTATE_OBJECTS, "Objects: Rotate"},
    {ACTION_SCALE_OBJECTS,  "Objects: Scale"},
    {ACTION_COPY_OBJECTS,   "Objects: Copy"},
    {ACTION_PASTE_OBJECTS,  "Objects: Paste"},
    {ACTION_HIDE_OBJECTS,   "Objects: Hide"},
    {ACTION_SHOW_OBJECTS,   "Objects: Show"},

    {ACTION_UNDO_COMMAND, "Undo command"},
    {ACTION_REDO_COMMAND, "Redo command"}
};

static_assert(sizeof action_name_map_data /
    sizeof(ActionNamePair) == TOTAL_ACTIONS);

static const ActionNameMap action_name_map(std::begin(action_name_map_data),
    std::end(action_name_map_data));

const char* to_c_string(Action action)
{
    return action_name_map.at(action);
}
