#include "KeyboardHandler.h"

#include "Action.h"
#include "commands/CommandList.h"
#include "EditorContext.h"
#include "KeyBinding.h"
#include "Settings.h"
#include "filesystem.h"

#include <fstream>
#include <vsg/ui/KeyEvent.h>

#include <cstdint>
#include <cstring>

#include <iostream>

static std::uint16_t get_byte_index(vsg::KeySymbol key)
{
    return key >> 3;
}

static std::uint8_t get_byte_value(vsg::KeySymbol key)
{
    return 1 << (key & 7);
}

KeyboardHandler::KeyboardHandler(EditorContext& context)
    : context(context)
{
    std::memset(key_state_bits, 0, 8192);
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_state_bits[get_byte_index(keyPress.keyBase)] |=
        get_byte_value(keyPress.keyBase);

    if (get_binding_state(ACTION_UNDO_COMMAND))
    {
        context.commands.undo();
    }
    else if (get_binding_state(ACTION_REDO_COMMAND))
    {
        context.commands.redo();
    }
    else if (get_binding_state(ACTION_SAVE_ROUTE))
    {
        const FileSystem& fs = FileSystem::getInstance();
        std::string dir_for_save = fs.combinePath(context.route_dir, "topology", "map");

        try
        {
            // Создаем резервную копия
            std::filesystem::copy_file(fs.combinePath(dir_for_save, "route1.map"),
                                       fs.combinePath(dir_for_save, "route1.map.prev"),
                                       std::filesystem::copy_options::overwrite_existing);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cout << e.what() << std::endl;
        }

        // Перезаписываем рабочую копию
        std::ofstream route_map_file(fs.combinePath(dir_for_save, "route1.map"));

        for (const auto& object : context.static_objects)
        {
            const vsg::dvec3& translation = object->get_translation();
            const vsg::dvec3 rotation_deg = -object->get_rotation_deg();

            route_map_file << object->label << "," << translation.x <<
                "," << translation.y << "," << translation.z << "," <<
                rotation_deg.x << "," << rotation_deg.y << "," <<
                rotation_deg.z << ";\n";
        }
    }
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_state_bits[get_byte_index(keyRelease.keyBase)] &=
        ~get_byte_value(keyRelease.keyBase);
}

bool KeyboardHandler::get_key_state(vsg::KeySymbol key) const
{
    return key_state_bits[get_byte_index(key)] & get_byte_value(key);
}

bool KeyboardHandler::get_any_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool KeyboardHandler::get_any_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) ||
        get_key_state(vsg::KEY_Control_R);
}

bool KeyboardHandler::get_any_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) || get_key_state(vsg::KEY_Alt_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    const KeyBinding key_binding = context.settings.key_bindings.at(action);

    if (!get_key_state(key_binding.key))
    {
        return false;
    }

    uint16_t modifiers = 0;
    modifiers |= (EDITOR_KEY_MODIFIER_SHIFT * get_any_shift_state());
    modifiers |= (EDITOR_KEY_MODIFIER_CTRL * get_any_ctrl_state());
    modifiers |= (EDITOR_KEY_MODIFIER_ALT * get_any_alt_state());

    return modifiers == key_binding.modifiers;
}
