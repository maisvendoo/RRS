#include "KeyBinding.h"

#include <vsg/ui/KeyEvent.h>

#include <map>
#include <vector>

const std::map<EditorKeyModifier, std::vector<vsg::KeySymbol>>
modifier_keys_map = {
    {EDITOR_KEY_MODIFIER_SHIFT_L, {vsg::KEY_Shift_L}},
    {EDITOR_KEY_MODIFIER_SHIFT_R, {vsg::KEY_Shift_R}},
    {EDITOR_KEY_MODIFIER_SHIFT_ANY, {vsg::KEY_Shift_L, vsg::KEY_Shift_R}},
    {EDITOR_KEY_MODIFIER_CTRL_L, {vsg::KEY_Control_L}},
    {EDITOR_KEY_MODIFIER_CTRL_R, {vsg::KEY_Control_R}},
    {EDITOR_KEY_MODIFIER_CTRL_ANY, {vsg::KEY_Control_L, vsg::KEY_Control_R}},
    {EDITOR_KEY_MODIFIER_ALT_L, {vsg::KEY_Alt_L}},
    {EDITOR_KEY_MODIFIER_ALT_R, {vsg::KEY_Alt_R}},
    {EDITOR_KEY_MODIFIER_ALT_ANY, {vsg::KEY_Alt_L, vsg::KEY_Alt_R}}
};
