#include "SelectObjectsCommand.h"

#include "RouteObject.h"

#include <cstdio>
#include <string>

SelectObjectsCommand::SelectObjectsCommand(const RouteObjects& objects)
    : objects(objects)
{
}

void SelectObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->select();
    }
}

void SelectObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->deselect();
    }
}

std::string SelectObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Select objects: %zu objects\n", objects.size());
    return buffer;
}
