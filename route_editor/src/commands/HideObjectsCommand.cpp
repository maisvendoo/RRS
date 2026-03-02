#include "HideObjectsCommand.h"

#include "RouteObject.h"

#include <cstdio>
#include <string>

HideObjectsCommand::HideObjectsCommand(EditorContext& context, const RouteObjects& objects)
    : Command(context)
    , objects(objects)
{
}

void HideObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->hide();
    }
}

void HideObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->show();
    }
}

std::string HideObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Hide objects: %zu objects\n", objects.size());
    return buffer;
}
