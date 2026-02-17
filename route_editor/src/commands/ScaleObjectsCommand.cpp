#include "ScaleObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

ScaleObjectsCommand::ScaleObjectsCommand(
    const std::vector<vsg::ref_ptr<RouteObject>>& objects,
    vsg::vec3 scale
)
    : objects(objects)
    , scale(scale)
{
}

void ScaleObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->scale(scale, true);
    }
}

void ScaleObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        object->scale(-scale, true);
    }
}

std::string ScaleObjectsCommand::to_string() const
{
    char buffer[64];
    std::snprintf(buffer, 64, "Scale objects: { %10.3f, %10.3f, %10.3f }",
        scale.x, scale.y, scale.z);
    return buffer;
}
