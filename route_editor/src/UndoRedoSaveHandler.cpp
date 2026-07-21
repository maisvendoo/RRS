#include "UndoRedoSaveHandler.h"

#include "Action.h"
#include "Journal.h"
#include "Keyboard.h"
#include "RouteObject.h"
#include "commands/CommandList.h"
#include "filesystem.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

UndoRedoSaveHandler::UndoRedoSaveHandler(
    vsg::ref_ptr<Keyboard>& keyboard,
    CommandList& commands,
    const std::string& route_dir,
    std::mutex& static_objects_mutex,
    const RouteObjects& static_objects
)
    : keyboard_{keyboard}
    , commands_{commands}
    , route_dir_{route_dir}
    , static_objects_mutex_{static_objects_mutex}
    , static_objects_{static_objects}
{
}

void UndoRedoSaveHandler::apply(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;

    if (keyboard_->pressed(ACTION_UNDO_COMMAND))
    {
        commands_.undo();
    }
    else if (keyboard_->pressed(ACTION_REDO_COMMAND))
    {
        commands_.redo();
    }
    else if (keyboard_->pressed(ACTION_SAVE_ROUTE))
    {
        save_route();
    }
}

void UndoRedoSaveHandler::save_route() const
{
    const FileSystem& fs{FileSystem::getInstance()};
    const std::string save_dir{fs.combinePath(route_dir_, "topology", "map")};

    try
    {
        // Создаём резервную копию
        std::filesystem::copy_file(
            fs.combinePath(save_dir, "route1.map"),
            fs.combinePath(save_dir, "route1.map.prev"),
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Journal::instance()->error(e.what());
    }

    // Перезаписываем рабочую копию
    std::ofstream route_map_file{fs.combinePath(save_dir, "route1.map")};

    std::lock_guard<std::mutex> lock_guard{static_objects_mutex_};
    for (const auto& object : static_objects_)
    {
        const vsg::dvec3& translation{object->get_translation()};
        const vsg::dvec3 rotation_deg{-object->get_rotation_deg()};

        route_map_file << object->label << "," <<
            translation.x << "," << translation.y << "," << translation.z << "," <<
            rotation_deg.x << "," << rotation_deg.y << "," << rotation_deg.z << ";\n";
    }
}
