#include "ScaleObjectsCommand.h"

#include "RouteObject.h"

#include <vsg/maths/vec3.h>

#include <cstdio>
#include <string>

ScaleObjectsCommand::ScaleObjectsCommand(EditorContext& context, const RouteObjects& objects,
    vsg::vec3 pivot, vsg::vec3 scale)
    : Command(context)
    , objects(objects)
    , pivot(pivot)
    , scale(scale)
{
}

void ScaleObjectsCommand::execute() const
{
    for (const auto& object : objects)
    {
        object->scale_relative_to_pivot(pivot, scale, object->matrix);
    }
}

void ScaleObjectsCommand::undo() const
{
    for (const auto& object : objects)
    {
        // TODO: Wrong calculation. Fix
        object->scale_relative_to_pivot(pivot, -scale, object->matrix);
    }
}

std::string ScaleObjectsCommand::to_string() const
{
    char buffer[128];
    std::snprintf(buffer, 128,
        "Scale objects: pivot = { %10.3f, %10.3f, %10.3f }\n"
        "               scale = { %10.3f, %10.3f, %10.3f }",
        pivot.x, pivot.y, pivot.z, scale.x, scale.y, scale.z);
    return buffer;
}
