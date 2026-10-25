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

DeleteObjectsCommand::DeleteObjectsCommand(EditorContext& editor_ctx)
    : Command(editor_ctx)
    , objects(editor_ctx.selected_objects)
{
    update_description();
}

void DeleteObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->deselect();

        editor_ctx.static_objects.lock()->remove(object);

        auto& route_children = editor_ctx.route->children;

        route_children.erase(
            std::remove_if(route_children.begin(), route_children.end(),
                [object](const vsg::Switch::Child& child) {
                    return child.node == object;
                }
            ),
            route_children.end()
        );
    }

    editor_ctx.compile_info_nodes.lock()->emplace_back(
        CompileInfoNode{editor_ctx.route});

    editor_ctx.gizmo->update_visibility();
}

void DeleteObjectsCommand::undo()
{
    auto compile_infos = editor_ctx.compile_info_switch_nodes.lock();
    auto static_objects = editor_ctx.static_objects.lock();

    for (const auto& object : objects)
    {
        compile_infos->emplace_back(CompileInfoSwitchNode{editor_ctx.route,
                                                          object,
                                                          vsg::MASK_ALL});

        static_objects->emplace_back(object);

        object->select();
    }

    editor_ctx.gizmo->update_visibility();
}

void DeleteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects", objects.size());
}
