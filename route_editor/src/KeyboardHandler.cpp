#include "KeyboardHandler.h"

#include "Action.h"
#include "EditorContext.h"
#include "Journal.h"
#include "KeyBinding.h"
#include "Settings.h"
#include "filesystem.h"
#include "commands/CommandList.h"

#include <vsg/ui/KeyEvent.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>

KeyboardHandler::KeyboardHandler(EditorContext& context)
    : context_(context)
{
}

static void save_route(
    const std::string& route_dir,
    std::mutex& static_objects_mutex,
    const RouteObjects& static_objects
)
{
    const FileSystem& fs = FileSystem::getInstance();
    std::string dir_for_save = fs.combinePath(route_dir, "topology", "map");

    try
    {
        // Создаем резервную копия
        std::filesystem::copy_file(
            fs.combinePath(dir_for_save, "route1.map"),
            fs.combinePath(dir_for_save, "route1.map.prev"),
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        Journal::instance()->error(e.what());
    }

    // Перезаписываем рабочую копию
    std::ofstream route_map_file(fs.combinePath(dir_for_save, "route1.map"));

    std::lock_guard<std::mutex> lock_guard(static_objects_mutex);
    for (const auto& object : static_objects)
    {
        const vsg::dvec3& translation = object->get_translation();
        const vsg::dvec3 rotation_deg = -object->get_rotation_deg();

        route_map_file << object->label << "," << translation.x <<
            "," << translation.y << "," << translation.z << "," <<
            rotation_deg.x << "," << rotation_deg.y << "," <<
            rotation_deg.z << ";\n";
    }
}

void KeyboardHandler::apply(vsg::KeyPressEvent& keyPress)
{
    key_state_bits_.set(keyPress.keyBase, true);

    // Move this logic somewhere else
    if (get_binding_state(ACTION_UNDO_COMMAND))
    {
        context_.commands.undo();
    }
    else if (get_binding_state(ACTION_REDO_COMMAND))
    {
        context_.commands.redo();
    }
    else if (get_binding_state(ACTION_SAVE_ROUTE))
    {
        save_route(context_.route_dir, context_.static_objects_mutex, context_.static_objects);
    }
}

void KeyboardHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    key_state_bits_.set(keyRelease.keyBase, false);
}

bool KeyboardHandler::get_key_state(vsg::KeySymbol key) const
{
    return key_state_bits_.test(key);
}

bool KeyboardHandler::get_shift_state() const
{
    return get_key_state(vsg::KEY_Shift_L) || get_key_state(vsg::KEY_Shift_R);
}

bool KeyboardHandler::get_ctrl_state() const
{
    return get_key_state(vsg::KEY_Control_L) ||
        get_key_state(vsg::KEY_Control_R);
}

bool KeyboardHandler::get_alt_state() const
{
    return get_key_state(vsg::KEY_Alt_L) || get_key_state(vsg::KEY_Alt_R);
}

bool KeyboardHandler::get_binding_state(Action action) const
{
    const KeyBinding key_binding = context_.settings.key_bindings.at(action);

    if (!get_key_state(key_binding.key))
    {
        return false;
    }

    uint16_t modifiers = 0;
    modifiers |= (vsg::MODKEY_Shift * get_shift_state());
    modifiers |= (vsg::MODKEY_Control * get_ctrl_state());
    modifiers |= (vsg::MODKEY_Alt * get_alt_state());

    return modifiers == key_binding.modifiers;
}
