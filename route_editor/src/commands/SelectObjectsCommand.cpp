#include "SelectObjectsCommand.h"

#include "Command.h"
#include "EditorContext.h"
#include "Gizmo.h"
#include "RouteObject.h"

#include <cstdio>

SelectObjectsCommand::SelectObjectsCommand(EditorContext& context)
    : Command(context)
{
}

void SelectObjectsCommand::execute()
{
    for (RouteObject* const object : objects_to_select)
    {
        object->select();
    }

    for (RouteObject* const object : objects_to_deselect)
    {
        object->deselect();
    }

    context.gizmo->update_visibility();
}

void SelectObjectsCommand::undo()
{
    for (RouteObject* const object : objects_to_select)
    {
        object->deselect();
    }

    for (RouteObject* const object : objects_to_deselect)
    {
        object->select();
    }

    context.gizmo->update_visibility();
}

void SelectObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Select objects: to select: %zu objects\n"
        "                to deselect: %zu objects",
        objects_to_select.size(), objects_to_deselect.size()
    );
}
