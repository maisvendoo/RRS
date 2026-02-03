#include "Action.h"

#include <cassert>

const char* to_c_string(Action action)
{
    assert(action >= 0);
    assert(action < TOTAL_ACTIONS);

    switch (action)
    {
        case ACTION_MOVE_CAMERA_FORWARD:
        {
            return "Move camera forward";
        }
        case ACTION_MOVE_CAMERA_BACKWARD:
        {
            return "Move camera backward";
        }
        case ACTION_MOVE_CAMERA_LEFT:
        {
            return "Move camera left";
        }
        case ACTION_MOVE_CAMERA_RIGHT:
        {
            return "Move camera right";
        }
        case ACTION_MOVE_OBJECTS:
        {
            return "Move objects";
        }
        default:
        {
            return "";
        }
    }
}
