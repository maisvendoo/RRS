#include "editor/commands/SelectObjects.h"

#include "editor/commands/Command.h"
#include "editor/RouteObject.h"

#include <cstdio>

SelectObjects::SelectObjects(EditorContext& context)
    : Command(context)
{
}

void SelectObjects::execute()
{
    for (const auto& object : objects_to_select)
    {
        object->select();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->deselect();
    }
}

void SelectObjects::undo()
{
    for (const auto& object : objects_to_select)
    {
        object->deselect();
    }

    for (const auto& object : objects_to_deselect)
    {
        object->select();
    }
}

void SelectObjects::update_description()
{
    std::snprintf(description_, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Select objects: to select: %zu objects\n"
        "              to deselect: %zu objects",
        objects_to_select.size(), objects_to_deselect.size()
    );
}
