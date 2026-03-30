#include "HideObjectsCommand.h"

#include "Command.h"
#include "RouteObject.h"

#include <cstdio>

HideObjectsCommand::HideObjectsCommand(EditorContext& context)
    : Command(context)
{
}

void HideObjectsCommand::execute()
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

void HideObjectsCommand::undo()
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

void HideObjectsCommand::update_description()
{
    std::snprintf(description, COMMAND_DESCRIPTION_BUFFER_SIZE,
        "Hide objects: to hide: %zu objects\n"
        "              to show: %zu objects",
        objects_to_hide.size(), objects_to_show.size()
    );
}
