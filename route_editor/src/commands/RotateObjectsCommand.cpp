#include "RotateObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

RotateObjectsCommand::RotateObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects,
    vsg::vec3 rotation_deg
)
    : objects(objects)
    , rotation_deg(rotation_deg)
{
}

void RotateObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->rotate(rotation_deg);
    }
}

void RotateObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->rotate(-rotation_deg);
    }
}

std::string RotateObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Rotate objects: { %10.3f, %10.3f, %10.3f }",
        rotation_deg.x, rotation_deg.y, rotation_deg.z);
    return buffer;
}
