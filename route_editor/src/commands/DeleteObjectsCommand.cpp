#include "editor/commands/DeleteObjectsCommand.h"

#include "editor/commands/Command.h"
#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/Route.h"
#include "editor/RouteObject.h"

#include <vsg/core/Mask.h>
#include <vsg/nodes/Switch.h>

#include <algorithm>
#include <cstdio>

DeleteObjectsCommand::DeleteObjectsCommand(EditorContext& editor_context)
    : Command(editor_context)
    , objects_(editor_context.selected_objects)
{
    update_description();
}

void DeleteObjectsCommand::execute()
{
    for (const auto& object : objects_)
    {
        object->deselect();

        editor_context.static_objects.lock()->remove(object);

        auto& route_children = editor_context.route->children;

        route_children.erase(
            std::remove_if(route_children.begin(), route_children.end(),
                [object](const vsg::Switch::Child& child) {
                    return child.node == object;
                }
            ),
            route_children.end()
        );
    }

    editor_context.compile_infos.lock()->emplace_back(CompileInfo{
        nullptr, editor_context.route});

    editor_context.gizmo->update_visibility();
}

void DeleteObjectsCommand::undo()
{
    auto compile_infos = editor_context.compile_infos.lock();
    auto static_objects = editor_context.static_objects.lock();

    for (const auto& object : objects_)
    {
        compile_infos->emplace_back(CompileInfo{
            editor_context.route, object, vsg::MASK_ALL});

        static_objects->emplace_back(object);

        object->select();
    }

    editor_context.gizmo->update_visibility();
}

void DeleteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects",
        objects_.size()
    );
}
