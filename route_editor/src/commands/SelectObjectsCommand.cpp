#include "editor/commands/SelectObjectsCommand.h"

#include "editor/EditorContext.h"
#include "editor/Gizmo.h"
#include "editor/StaticObjectManager.h"
#include "editor/commands/Command.h"

#include <cstddef>
#include <cstdio>

SelectObjectsCommand::SelectObjectsCommand(EditorContext& context)
    : Command(context)
    , static_object_manager(*editor_ctx.static_object_manager)
    , gizmo(*editor_ctx.gizmo)
{
}

void SelectObjectsCommand::execute()
{
    for (const std::size_t id : ids_to_deselect)
    {
        static_object_manager.deselect_object(id);
    }

    for (const std::size_t id : ids_to_select)
    {
        static_object_manager.select_object(id);
    }

    gizmo.update_visibility();
}

void SelectObjectsCommand::undo()
{
    for (const std::size_t id : ids_to_select)
    {
        static_object_manager.deselect_object(id);
    }

    for (const std::size_t id : ids_to_deselect)
    {
        static_object_manager.select_object(id);
    }

    gizmo.update_visibility();
}

void SelectObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
                  "Select objects: to select: %zu objects\n"
                  "              to deselect: %zu objects",
                  ids_to_select.size(), ids_to_deselect.size());
}
