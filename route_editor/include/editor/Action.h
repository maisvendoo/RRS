#ifndef EDITOR_ACTION_H
#define EDITOR_ACTION_H

#include <array>
#include <vsg/ui/KeyEvent.h>

#include <cstdint>
#include <string>

class CfgReader;

enum Action
{
    ACTION_MOVE_CAMERA_FORWARD,
    ACTION_MOVE_CAMERA_BACKWARD,
    ACTION_MOVE_CAMERA_LEFT,
    ACTION_MOVE_CAMERA_RIGHT,
    ACTION_TRANSLATE_OBJECTS,
    ACTION_ROTATE_OBJECTS,
    ACTION_SCALE_OBJECTS,
    ACTION_COPY_OBJECTS,
    ACTION_PASTE_OBJECTS,
    ACTION_HIDE_OBJECTS,
    ACTION_SHOW_OBJECTS,
    ACTION_DELETE_OBJECTS,
    ACTION_UNDO_COMMAND,
    ACTION_REDO_COMMAND,
    ACTION_SAVE_ROUTE,
    ACTION_SWAP_PROJECTION_MATRIX,
    TOTAL_ACTIONS
};

struct Actions
{
    std::array<std::string, TOTAL_ACTIONS> descriptions;
    std::array<std::string, TOTAL_ACTIONS> setting_names;
    std::array<vsg::KeySymbol, TOTAL_ACTIONS> keys;
    std::array<std::uint16_t, TOTAL_ACTIONS> modifiers;

    int active_action = -1;

    Actions();

    void read(CfgReader& cfg);
};

#endif // EDITOR_ACTION_H
