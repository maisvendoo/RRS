#include "MoveObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>
#include <string>

MoveObjectsCommand::MoveObjectsCommand(const RouteObjects& objects,
    vsg::vec3 translation)
    : objects(objects)
    , translation(translation)
{
}

void MoveObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->move(translation, true);
    }
}

void MoveObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->move(-translation, true);
    }
}

std::string MoveObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Move objects: { %10.3f, %10.3f, %10.3f }",
        translation.x, translation.y, translation.z);
    return buffer;
}
