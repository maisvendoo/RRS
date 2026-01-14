#include "Action.h"

#include <cassert>

const char* to_c_string(Action action)
{
    assert(action < ACTION_TOTAL_COUNT);

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
        default:
        {
            return "";
        }
    }
}
