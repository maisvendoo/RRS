#include "MoveObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <cstdio>
#include <string>
#include <vector>

MoveObjectsCommand::MoveObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects,
    vsg::vec3 translation
)
    : objects(objects)
    , translation(translation)
{
}

void MoveObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->move(translation);
    }
}

void MoveObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->move(-translation);
    }
}

std::string MoveObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Move objects: { %10.3f, %10.3f, %10.3f }",
        translation.x, translation.y, translation.z);
    return buffer;
}
