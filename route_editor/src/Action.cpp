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
            return "Camera: move forward";
        }
        case ACTION_MOVE_CAMERA_BACKWARD:
        {
            return "Camera: move backward";
        }
        case ACTION_MOVE_CAMERA_LEFT:
        {
            return "Camera: move left";
        }
        case ACTION_MOVE_CAMERA_RIGHT:
        {
            return "Camera: move right";
        }
        case ACTION_MOVE_OBJECTS:
        {
            return "Object: Grab";
        }
        case ACTION_ROTATE_OBJECTS:
        {
            return "Object: Rotate";
        }
        default:
        {
            return "";
        }
    }
}
