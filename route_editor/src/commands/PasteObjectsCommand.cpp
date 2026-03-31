#include "PasteObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "Route.h"
#include "RouteObject.h"

#include <vsg/app/Viewer.h>
#include <vsg/core/Mask.h>

#include <algorithm>
#include <cstdio>

PasteObjectsCommand::PasteObjectsCommand(EditorContext& context)
    : Command(context)
    , objects_to_paste(context.copied_objects)
    , objects_to_deselect(context.selected_objects)
{
    update_description();
}

void PasteObjectsCommand::execute()
{
    for (const auto& object : objects_to_deselect)
    {
        object->deselect();
    }

    if (pasted_objects.empty())
    {
        for (const auto& object : objects_to_paste)
        {
            pasted_objects.emplace_back(object->copy());
        }
    }

    for (const auto& pasted_object : pasted_objects)
    {
        context.route->addChild(vsg::MASK_ALL, pasted_object);
        context.objects.emplace_back(pasted_object);

        pasted_object->select();
    }

    const auto compile_result = context.viewer->compileManager->compile(
        context.route);

    vsg::updateViewer(*context.viewer, compile_result);

    context.gizmo->update_visibility();
}

void PasteObjectsCommand::undo()
{
    for (const auto& pasted_object : pasted_objects)
    {
        pasted_object->deselect();

        RouteObjects& objects = context.objects;
        objects.erase(std::find(objects.begin(), objects.end(), pasted_object));

        const auto route = context.route;

        for (auto it = route->children.begin(); it != route->children.end();
            ++it)
        {
            if (it->node == pasted_object)
            {
                route->children.erase(it);
                break;
            }
        }
    }

    for (const auto& object : objects_to_deselect)
    {
        object->select();
    }

    const auto compile_result = context.viewer->compileManager->compile(
        context.route);

    vsg::updateViewer(*context.viewer, compile_result);

    context.gizmo->update_visibility();
}

void PasteObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Paste objects: to paste: %zu objects",
        objects_to_paste.size()
    );
}
