#ifndef ACTION_H
#define ACTION_H

enum Action
{
    ACTION_MOVE_CAMERA_FORWARD,
    ACTION_MOVE_CAMERA_BACKWARD,
    ACTION_MOVE_CAMERA_LEFT,
    ACTION_MOVE_CAMERA_RIGHT,
    TOTAL_ACTIONS
};

const char* to_c_string(Action action);

#endif // ACTION_H
