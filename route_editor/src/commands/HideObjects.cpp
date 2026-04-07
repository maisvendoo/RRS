#include "commands/HideObjects.h"

#include "commands/Command.h"
#include "RouteObject.h"

#include <cstdio>

HideObjects::HideObjects(EditorContext& context)
    : Command(context)
{
}

void HideObjects::execute()
{
    for (const auto& object : objects_to_hide)
    {
        object->hide();
    }

    for (const auto& object : objects_to_show)
    {
        object->show();
    }
}

void HideObjects::undo()
{
    for (const auto& object : objects_to_hide)
    {
        object->show();
    }

    for (const auto& object : objects_to_show)
    {
        object->hide();
    }
}

void HideObjects::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Hide objects: to hide: %zu objects\n"
        "              to show: %zu objects",
        objects_to_hide.size(), objects_to_show.size()
    );
}
