#include "DeselectObjectsCommand.h"

#include "RouteObject.h"

#include <cstdio>
#include <string>

DeselectObjectsCommand::DeselectObjectsCommand(const RouteObjects& objects)
    : objects(objects)
{
}

void DeselectObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->deselect();
    }
}

void DeselectObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->select();
    }
}

std::string DeselectObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Deselect objects: %zu objects\n", objects.size());
    return buffer;
}
