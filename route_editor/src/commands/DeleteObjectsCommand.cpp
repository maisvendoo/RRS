#include "commands/DeleteObjectsCommand.h"

#include "commands/Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"

#include <vsg/core/Mask.h>

#include <algorithm>
#include <cstdio>

DeleteObjectsCommand::DeleteObjectsCommand(EditorContext& context)
    : Command(context)
    , objects(context.selected_objects)
{
    update_description();
}

void DeleteObjectsCommand::execute()
{
    for (const auto& object : objects)
    {
        object->deselect();

        context.static_objects.erase(std::find(context.static_objects.begin(),
            context.static_objects.end(), object));

        const auto route = context.route;

        for (auto it = route->children.begin(); it != route->children.end();
            ++it)
        {
            if (it->node == object)
            {
                route->children.erase(it);
                break;
            }
        }
    }

    context.compile_infos.emplace_back(CompileInfo{nullptr, context.route});

    context.gizmo->update_visibility();
}

void DeleteObjectsCommand::undo()
{
    for (const auto& object : objects)
    {
        context.compile_infos.emplace_back(CompileInfo{
            context.route, object, vsg::MASK_ALL});

        context.static_objects.emplace_back(object);

        object->select();
    }

    context.gizmo->update_visibility();
}

void DeleteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects",
        objects.size()
    );
}
