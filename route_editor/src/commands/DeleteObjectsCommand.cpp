#include "DeleteObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"

#include <vsg/app/Viewer.h>

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

        // context.objects.erase(std::find(context.objects.begin(),
            // context.objects.end(), object));

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

    const auto compile_result = context.viewer->compileManager->compile(
        context.route);

    vsg::updateViewer(*context.viewer, compile_result);

    context.gizmo->update_visibility();
}

void DeleteObjectsCommand::undo()
{
    for (const auto& object : objects)
    {
        context.route->addChild(vsg::MASK_ALL, object);
        // context.objects.emplace_back(object);

        object->select();
    }

    const auto compile_result = context.viewer->compileManager->compile(
        context.route);

    vsg::updateViewer(*context.viewer, compile_result);

    context.gizmo->update_visibility();
}

void DeleteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Delete objects: to delete: %zu objects",
        objects.size()
    );
}
