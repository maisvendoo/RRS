#include "editor/commands/SelectObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/RouteObject.h"
#include "editor/commands/Command.h"

#include <cstdio>

SelectObjectsCommand::SelectObjectsCommand(EditorContext& context)
    : Command(context)
{
}

void SelectObjectsCommand::execute()
{
    for (const auto& object : objects_to_select)
    {
        object->select();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->deselect();
    }

    editor_context.gizmo->update_visibility();
}

void SelectObjectsCommand::undo()
{
    for (const auto& object : objects_to_select)
    {
        object->deselect();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->select();
    }

    editor_context.gizmo->update_visibility();
}

void SelectObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Select objects: to select: %zu objects\n"
        "              to deselect: %zu objects",
        objects_to_select.size(), objects_to_deselect.size()
    );
}
