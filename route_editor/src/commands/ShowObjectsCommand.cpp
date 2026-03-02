#include "ShowObjectsCommand.h"

#include "RouteObject.h"

#include <cstdio>
#include <string>

ShowObjectsCommand::ShowObjectsCommand(EditorContext& context, const RouteObjects& objects)
    : Command(context)
    , objects(objects)
{
}

void ShowObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->show();
    }
}

void ShowObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->hide();
    }
}

std::string ShowObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Show objects: %zu objects\n", objects.size());
    return buffer;
}
